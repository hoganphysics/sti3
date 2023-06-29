#ifndef STI_NETWORK_REMOTEIMAGEWRITER_H
#define STI_NETWORK_REMOTEIMAGEWRITER_H

#include <sti/utils/ImageWriter.h>


namespace STI
{
namespace Network
{

class RemoteImageWriter : public STI::Utils::ImageWriter
{
public:

    RemoteImageWriter(const std::shared_ptr<STI::Utils::FileHolder>& imageFile)
    : imageFile(imageFile) {}
    ~RemoteImageWriter() {}

    void clear() {}
    void addImage(STI::Utils::Image* image) {}
    bool write(const std::string& targetDirectory, std::shared_ptr<STI::Utils::FileHolder>& fileHolder)
    {
        fileHolder = imageFile;
        return fileHolder != 0;
    }

private:

    std::shared_ptr<STI::Utils::FileHolder> imageFile;

};


} //Network
} //STI

#endif
