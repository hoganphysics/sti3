#include <sti/device/ImportedFile.h>

using STI::Device::ImportedFile;

ImportedFile::ImportedFile(const std::string& importID,
                           const STI::Utils::FileID& fileID,
                           ReleaseCallback releaseCallback)
: importID(importID),
  fileID(fileID),
  releaseCallback(releaseCallback),
  closed(false)
{
}

ImportedFile::~ImportedFile()
{
    close();
}

const std::string& ImportedFile::getImportID() const
{
    return importID;
}

const STI::Utils::FileID& ImportedFile::getFileID() const
{
    return fileID;
}

bool ImportedFile::close()
{
    if (closed) {
        return true;
    }

    bool success = true;
    if (releaseCallback) {
        success = releaseCallback(importID);
    }

    if (success) {
        closed = true;
    }
    return success;
}

bool ImportedFile::isClosed() const
{
    return closed;
}
