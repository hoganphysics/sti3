#include <sti/utils/Image.h>

#include <sti/utils/BinaryData.h>
#include <sti/utils/ImageWriter.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileServer.h>

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


//Image::Image(const std::string& filename, const std::shared_ptr<ImageWriter>& writer)
//: isChild_(false), customWriter(writer)
//{
//    setFilename(filename);

    // if extension does not match known extensions
    // DefaultImageWritter::isSupported(extension_)
    // check for customWritter
    // default to know type?
//}

Image::~Image()
{
}

//std::shared_ptr<Image> Image::makeChildImage()
//{
//    auto child = std::make_shared<Image>(filename_);
//
//    children.push_back(child);
//
//    return child;
//}

Image& Image::setFilename(const std::string& filename)
{
    //std::filesystem::path rawFilename(filename);
    //filename_ = rawFilename.stem().string();
    //extension_ = rawFilename.extension().string();
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

//void Image::setWriter(const std::shared_ptr<ImageWriter>& writer)
//{
//    customWriter = writer;
//}

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
//
//void Image::writeToFile(const std::shared_ptr<ImageWriter>& writer, const std::string& targetDirectory)
//{
//    if (isChild_) return;
//    if (writer == 0 && customWriter == 0) return;
//
//    std::shared_ptr<ImageWriter> imageWriter;
//    if (customWriter != 0) {
//        imageWriter = customWriter;
//    }
//    else {
//        imageWriter = writer;
//    }
//
//    if (fileHolder.get() != 0 && fileHolder.get()->exists() ) return;  //already written
//
//    imageWriter->clear();
//    imageWriter->addImage(this);
//
//    for (auto& child : children) {
//        if (child != 0) {
//            imageWriter->addImage(child.get());
//        }
//    }
//
//    std::shared_ptr<FileHolder> file;
//    
//    if (imageWriter->write(targetDirectory, file) && (file != 0)) {
//        fileHolder.set(file);
//    }
//}

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

//bool Image::isChild() const
//{
//    return isChild_;
//}
//
//const std::vector<std::shared_ptr<Image>>& Image::getChildren() const
//{
//    return children;
//}

bool Image::operator==(const Image& other) const
{
    return other.getFileID() == fileID;
}

bool Image::operator!=(const Image& other) const
{
    return !((*this) == other);
}

// template<class Archive>
// void Image::serialize(Archive& archive)
// {
// 	archive(
// 		cereal::make_nvp("filename", filename_),
//         cereal::make_nvp("extension", extension_),
//         cereal::make_nvp("height", height_),
//         cereal::make_nvp("width", width_),
//         cereal::make_nvp("children", children),
//         cereal::make_nvp("isChild", isChild_),
//         // cereal::make_nvp("imageData", imageData)
//         cereal::make_nvp("fileHolder", fileHolder)
// 		);
//     // CachedValue<std::shared_ptr<BinaryData>> imageData;
//     // CachedValue<std::shared_ptr<FileHolder>> fileHolder;
// }

// template void Image::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
// template void Image::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );



template<class Archive>
void Image::save(Archive& archive) const
{
    archive(
		cereal::make_nvp("fileID", fileID),
        cereal::make_nvp("height", height_),
        cereal::make_nvp("width", width_),
        cereal::make_nvp("imageData", imageData.get())
        // cereal::make_nvp("fileID", fileHolder->getID())
        // cereal::make_nvp("fileHolder", fileHolder.get())
	);
}

template void Image::save<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&) const;

template void Image::save<cereal::JSONOutputArchive>( cereal::JSONOutputArchive& ) const;



template<class Archive>
void Image::load(Archive& archive)
{
    std::shared_ptr<BinaryData> bin;
    // STI::Utils::FileID fileID;

    archive(
		cereal::make_nvp("fileID", fileID),
        cereal::make_nvp("height", height_),
        cereal::make_nvp("width", width_),
        cereal::make_nvp("imageData", bin)
        // cereal::make_nvp("fileID", fileID)
        // cereal::make_nvp("fileHolder", fileHolder)
	);
    imageData.set(bin);
}

template void Image::load<cereal::XMLInputArchive>(cereal::XMLInputArchive&);

template void Image::load<cereal::JSONInputArchive>( cereal::JSONInputArchive& );

