#include <sti/utils/Image.h>

#include <sti/utils/BinaryData.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/LocalFileHolder.h>

#include <algorithm>
#include <filesystem>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

using STI::Utils::Image;
using STI::Utils::MixedValue;
using STI::Utils::FileServer;


Image::Image()
{
}

Image::Image(const std::string& orginID, const std::string& filename)
{
    std::filesystem::path filepath = filename;

    fileID.filename = filepath.filename().string();
    fileID.origin = orginID;
    fileID.persistenceLocation = orginID;
    fileID.path = filepath.parent_path().string();
}


Image::Image(const FileID& fileID)
: fileID(fileID)
{
}

Image::~Image()
{
}

Image::Image(const Image& other)
{
    fileID = other.fileID;

    height_ = other.height_;
    width_ = other.width_;

    metaData = other.metaData;

    if (other.imageData.isCached()) {
        imageData.set(other.imageData.get());
    }
    
    if (other.fileHolder.isCached()) {
        fileHolder.set(other.fileHolder.get());
    }
}


Image& Image::setFilename(const std::string& filename)
{

    fileID.filename = filename;

    return (*this);
}

void Image::setFileID(const STI::Utils::FileID& id)
{
    fileID = id;
}

Image& Image::setHeight(unsigned height)
{
    height_ = height;

    MixedValue value;
    value.setValue(height);

    setMetaData("height", value);
    return (*this);
}

Image& Image::setWidth(unsigned width)
{
    width_ = width;

    MixedValue value;
    value.setValue(width);

    setMetaData("width", value);
    return (*this);
}

Image& Image::setMetaData(const std::string& key, const MixedValue& value)
{
    metaData.addMetaData(key, value);
    return (*this);
}

void Image::setImageData(const std::shared_ptr<FileHolder>& file)
{
    if (file == 0) return;

    fileHolder.set(file);

    auto id = file->getID();
    fileID.origin = id.origin;
    fileID.persistenceLocation = id.persistenceLocation;
    fileID.path = id.path;
}

void Image::setImageData(const std::shared_ptr<BinaryData>& data)
{
    if (data == 0) return;

    imageData.set(data);
}


bool Image::write(const std::shared_ptr<FileServer>& sourceFileServer, const std::shared_ptr<FileHolder>& destination)
{
    if (sourceFileServer == 0 || destination == 0) return false;

    bool success = false;

    if (fileHolder.isCached() && fileHolder.get() != 0) {
        
        success = sourceFileServer->transferFile(fileHolder.get()->getID(), destination, STI::Utils::FileTransferType::Binary);
    }
    else if (imageData.isCached() && imageData.get() != 0) {
        destination->openFile();

        char* data;
        imageData.get()->getBytes(data, false);  //keep ownership
        success = destination->write(data, imageData.get()->bytes());

        destination->closeFile();
        
        auto destinationFileID = destination->getID();
        
        fileID.filename = destinationFileID.filename;
        fileID.path = destinationFileID.path;
        fileID.persistenceLocation = destinationFileID.persistenceLocation;
        
        fileHolder.set(destination);
    }
    return success;
}

bool Image::saveToFile()
{
    if (fileHolder.isCached() && fileHolder.get() != 0 && fileHolder.get()->exists()) {
        return true;
    }

    auto file = std::make_shared<STI::Utils::LocalFileHolder>(fileID.origin, fileID.path, fileID.filename);

    if (imageData.isCached() && imageData.get() != 0) {
        if (file->write(imageData.get())) {
            fileHolder.set(file);
            return true;
        }
    }
    
    return false;
}


bool Image::getData(std::shared_ptr<BinaryData>& data) const
{
    return imageData.isCached() && imageData.getValue(data) && (data != 0);
}

bool Image::getFile(std::shared_ptr<FileHolder>& file) const
{
    return fileHolder.isCached() && fileHolder.getValue(file) && (file != 0);
}

STI::Utils::FileID Image::getFileID() const
{
    return fileID;
}

unsigned Image::getHeight() const
{
    return height_;
}

unsigned Image::getWidth() const
{
    return width_;
}

bool Image::operator==(const Image& other) const
{
    return other.getFileID() == fileID;
}

bool Image::operator!=(const Image& other) const
{
    return !((*this) == other);
}

template<class Archive>
void Image::save(Archive& archive) const
{
    //Need to save any BinaryData to a file before serialization
    Image clone(*this); //make clone (shallow copy) to allow call to saveToFile()
    clone.saveToFile();

    archive(
		cereal::make_nvp("fileID", fileID),
        cereal::make_nvp("height", height_),
        cereal::make_nvp("width", width_),
        cereal::make_nvp("metaData", metaData.getMetaData())
	);
}

template void Image::save<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&) const;
template void Image::save<cereal::JSONOutputArchive>( cereal::JSONOutputArchive& ) const;


template<class Archive>
void Image::load(Archive& archive)
{  
    MixedValue metaDataValue;

    archive(
		cereal::make_nvp("fileID", fileID),
        cereal::make_nvp("height", height_),
        cereal::make_nvp("width", width_),
        cereal::make_nvp("metaData", metaDataValue)
	);

    metaData.merge(metaDataValue);
}

template void Image::load<cereal::XMLInputArchive>(cereal::XMLInputArchive&);
template void Image::load<cereal::JSONInputArchive>( cereal::JSONInputArchive& );

