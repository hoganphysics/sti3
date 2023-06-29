
#include "DefaultImageWriter.h"

#include <sti/utils/Image.h>
#include <sti/utils/LocalFileHolder.h>
#include <sti/utils/utils.h>

#include <filesystem>

using STI::Utils::DefaultImageWriter;
using STI::Utils::FileHolder;
using STI::Utils::LocalFileHolder;


DefaultImageWriter::DefaultImageWriter(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
: fileHolderFactory(factory)
{
}

DefaultImageWriter::~DefaultImageWriter()
{
}

void DefaultImageWriter::clear()
{
    images.clear();
}

void DefaultImageWriter::addImage(Image* image)
{
    images.push_back(image);
}

bool DefaultImageWriter::write(const std::string& targetDirectory, std::shared_ptr<FileHolder>& fileHolder)
{
    if (images.size() == 0) {
        return false;
    }

    std::filesystem::path fullFilename(targetDirectory);

    fullFilename /= (images.at(0)->getFilename() + "." + images.at(0)->getExtension());

    auto uniqueFilename = STI::Utils::makeUniquePath( fullFilename.string() );

    auto localFileHolder = fileHolderFactory->makeFileHolder(uniqueFilename);

    //write file here.  switch type based on extension

    return (localFileHolder != 0);
}

