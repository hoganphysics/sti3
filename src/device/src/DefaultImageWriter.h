#ifndef STI_UTILS_DEFAULTIMAGEWRITTER_H
#define STI_UTILS_DEFAULTIMAGEWRITTER_H


#include <sti/utils/FileHolderFactory.h>
#include <sti/utils/ImageWriter.h>

#include <string>
#include <memory>
#include <vector>


namespace STI
{
namespace Utils
{

class DefaultImageWriter : public ImageWriter
{
public:

    DefaultImageWriter(const std::shared_ptr<FileHolderFactory>& factory);
    ~DefaultImageWriter();

    void clear();
    void addImage(Image* image);
    bool write(const std::string& targetDirectory, std::shared_ptr<FileHolder>& fileHolder);

private:

    std::vector<Image*> images;

    std::shared_ptr<FileHolderFactory> fileHolderFactory;
};


} //Utils
} //STI

#endif
