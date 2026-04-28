#ifndef STI_UTILS_LOCALFILEHOLDER_H
#define STI_UTILS_LOCALFILEHOLDER_H

#include <sti/utils/CachedValue.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileHolderFactory.h>
#include <sti/utils/FileServer.h>

#include <string>
#include <memory>
#include <mutex>


namespace STI
{
namespace Utils
{

class BinaryData;
class VirtualFileHolder;

class LocalFileHolder : public FileHolder
{
public:

    LocalFileHolder(const std::string& originID, const std::string& path, const std::string& filename);
    virtual ~LocalFileHolder();

    bool operator==(const LocalFileHolder& other) const;
	bool operator!=(const LocalFileHolder& other) const;

    FileID getID() const;

    std::string getFilename() const;

    unsigned getFileSize() const;

    bool exists() const;

    bool transferFile(const std::shared_ptr<FileHolder>& destination);

    unsigned maxBufferSize() const;

    std::string md5Checksum();

	// bool openFileType(FileTransferType type, unsigned offset, unsigned& maxBufferSize);
    virtual bool openFile();

    bool write(const char* buffer, unsigned length);
    bool write(const std::shared_ptr<BinaryData>& data);
    // bool writeString(const std::string& buffer);
    virtual void closeFile();
    
    static bool makeMD5hash(std::istream& ifs, std::string& md5string, unsigned bufferSize);

protected:
    
    friend class LocalFileHolderFactory;
    
    LocalFileHolder(const std::string& originID, const FileID& id);

    virtual std::ostream* getostream();
    virtual bool getistream(std::shared_ptr<std::istream>& istream);

private:

    FileID fileID;

    STI::Utils::CachedValue<std::string> md5hash;

    std::unique_ptr<std::ofstream> ofs;
    
    mutable std::mutex fileMutex;
};


class LocalFileHolderFactory : public FileHolderFactory
{
public:

    LocalFileHolderFactory(const std::string& originID);

    std::shared_ptr<FileHolder> makeFileHolder(const std::string& path, const std::string& filename);
    std::shared_ptr<FileHolder> makeVirtualFileHolder(const FileID& fileID);
    std::shared_ptr<FileHolder> makeVirtualFileHolder(const std::shared_ptr<VirtualFileHolder>& backingHolder);

    std::string originID;
};

} //Utils
} //STI

#endif
