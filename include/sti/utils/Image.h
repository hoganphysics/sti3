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

class MixedValue;
class FileServer;

class Image
{
public:

    Image();    //serialization
    Image(const std::string& orginID, const std::string& filename);
    Image(const FileID& fileID);
    virtual ~Image();

    Image(const Image& other);

    FileID getFileID() const;
    void setFileID(const FileID& fileID);
    Image& setFilename(const std::string& filename);

    Image& setHeight(unsigned height);
    Image& setWidth(unsigned width);

    Image& setMetaData(const std::string& key, const MixedValue& value);

    void setImageData(const std::shared_ptr<FileHolder>& file);
    void setImageData(const std::shared_ptr<BinaryData>& data);

    // Keep lazily exported transport data alive for as long as this image can
    // be referenced by a remote caller.
    void retainDataForTransfer(const std::shared_ptr<BinaryData>& data) const;

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

    bool write(const std::shared_ptr<FileServer>& sourceFileServer, const std::shared_ptr<FileHolder>& destination);
    bool saveToFile();  //before serialization

    bool getData(std::shared_ptr<BinaryData>& data) const;
    bool getFile(std::shared_ptr<FileHolder>& file) const;

    unsigned getHeight() const;
    unsigned getWidth() const;

    bool operator==(const Image& other) const;
    bool operator!=(const Image& other) const;

    template<class Archive>
    void save(Archive& archive) const;

    template<class Archive>
    void load(Archive& archive);

private:

    CachedValue<std::shared_ptr<BinaryData>> imageData;
    CachedValue<std::shared_ptr<FileHolder>> fileHolder;
    mutable std::vector<std::shared_ptr<BinaryData>> retainedTransferData;

    FileID fileID;

    unsigned height_;
    unsigned width_;
};


} //Utils
} //STI

#endif
