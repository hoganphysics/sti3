#ifndef STI_UTILS_IMAGE_H
#define STI_UTILS_IMAGE_H

#include <sti/utils/BinaryData.h>
#include <sti/utils/CachedValue.h>
#include <sti/utils/MetaData.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileID.h>

#include <string>
#include <memory>
#include <vector>


namespace STI
{
namespace Utils
{

//class ImageWriter;
class MixedValue;
class FileServer;

class Image
{
public:

    Image();    //serialization
    Image(const std::string& orginID, const std::string& filename);
    //Image(const std::string& filename);
    Image(const FileID& fileID);
    //Image(const std::string& filename, const std::shared_ptr<ImageWriter>& writter);
    virtual ~Image();

    //std::shared_ptr<Image> makeChildImage();

    FileID getFileID() const;
    void setFileID(const FileID& fileID);

    //image dimensions

    Image& setFilename(const std::string& filename);

    Image& setHeight(unsigned height);
    Image& setWidth(unsigned width);

    Image& setMetaData(const std::string& key, const MixedValue& value);

    //void setWriter(const std::shared_ptr<ImageWriter>& writer);  //custom writer

    void setImageData(const std::shared_ptr<FileHolder>& file);

    void setImageData(const std::shared_ptr<BinaryData>& data);

    template<typename T>
    void setImageData(T*& data, size_t length, bool takeOwnership=true)
    {
        auto binaryData = std::make_shared<BinaryData>();

        if (binaryData != 0) {
            binaryData->assign<T>(data, length, takeOwnership);
            imageData.set(binaryData);
        }
    }

    MetaData metaData;

    //void writeToFile(const std::shared_ptr<ImageWriter>& writer, const std::string& targetDirectory);

    bool write(const std::shared_ptr<FileServer>& sourceFileServer, const std::shared_ptr<FileHolder>& destination);

    bool getData(std::shared_ptr<BinaryData>& data) const;
    bool getFile(std::shared_ptr<FileHolder>& file) const;

    unsigned getHeight() const;
    unsigned getWidth() const;

    //bool isChild() const;
    //const std::vector<std::shared_ptr<Image>>& getChildren() const;

    bool operator==(const Image& other) const;
    bool operator!=(const Image& other) const;

    // template<class Archive>
	// void serialize(Archive& archive);

    template<class Archive>
    void save(Archive& archive) const;

    template<class Archive>
    void load(Archive& archive);

private:

    CachedValue<std::shared_ptr<BinaryData>> imageData;
    CachedValue<std::shared_ptr<FileHolder>> fileHolder;

    // std::shared_ptr<BinaryData> thumbnail;

    //std::shared_ptr<ImageWriter> customWriter;

    //std::string filename_;
    //std::string extension_;     //tif, png, etc.

    FileID fileID;

    unsigned height_;
    unsigned width_;

    //std::vector<std::shared_ptr<Image>> children;

    //bool isChild_;   //child images are written by the parent image

};


} //Utils
} //STI

#endif

