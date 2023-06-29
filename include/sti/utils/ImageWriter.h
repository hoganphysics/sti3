#ifndef STI_UTILS_IMAGEWRITER_H
#define STI_UTILS_IMAGEWRITER_H


#include <sti/utils/FileHolder.h>

#include <string>
#include <memory>


namespace STI
{
namespace Utils
{

class Image;

class ImageWriter
{
public:

    virtual void clear() = 0;
    virtual void addImage(Image* image) = 0;
    virtual bool write(const std::string& targetDirectory, std::shared_ptr<FileHolder>& fileHolder) = 0;
};


} //Utils
} //STI

#endif
