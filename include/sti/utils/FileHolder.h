#ifndef STI_UTILS_FILEHOLDER_H
#define STI_UTILS_FILEHOLDER_H

#include <sti/utils/FileID.h>

#include <string>
#include <memory>


namespace STI
{
namespace Utils
{


class FileHolder
{
public:

    virtual ~FileHolder() {}

    bool operator==(const FileHolder& other) const
    {
        return other.getFilename() == getFilename(); // && other.md5Checksum() == md5Checksum();
    }
    bool operator!=(const FileHolder& other) const
    {
        return !((*this) == other);
    }

    virtual FileID getID() const = 0;
    virtual std::string getFilename() const = 0;

    virtual unsigned getFileSize() const = 0;

    virtual bool exists() const = 0;

    virtual bool transferFile(const std::shared_ptr<FileHolder>& destination) = 0;
    virtual unsigned maxBufferSize() const = 0;

    // virtual bool deleteFile() = 0;

    virtual std::string md5Checksum() = 0;
    
    virtual bool write(const char* buffer, unsigned length) = 0;
	// virtual bool writeString(const std::string& buffer) = 0;

	// virtual bool openFileType(FileTransferType type, unsigned offset, unsigned& maxBufferSize) = 0;

    virtual bool openFile() = 0;    //before writing
    virtual void closeFile() = 0;

};


} //Utils
} //STI

#endif
