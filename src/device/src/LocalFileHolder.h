
#ifndef STI_UTILS_LOCALFILEHOLDER_H
#define STI_UTILS_LOCALFILEHOLDER_H

#include <sti/utils/FileHolder.h>
#include <sti/utils/FileHolderFactory.h>

#include <string>
#include <memory>
#include <mutex>

namespace STI
{
namespace Utils
{


class LocalFileHolder : public FileHolder
{
public:

    LocalFileHolder();    //for serialization

    virtual ~LocalFileHolder();

    std::string getFilename() const;

    bool exists() const;

    bool transferFile(const std::shared_ptr<FileHolder>& destination);

    unsigned maxBufferSize() const;

    bool deleteFile();

    std::string md5Checksum();

    bool openFile();
    bool write(const char* buffer, unsigned length);
    void closeFile();
    
    static bool makeMD5hash(const std::string& fname, std::string& md5string, unsigned bufferSize);

   	template<class Archive>
	void serialize(Archive& archive);

protected:
    
    friend class LocalFileHolderFactory;
    LocalFileHolder(const std::string& filename);

private:

    std::string filename;
    std::string md5hash;
    bool hashed;

    std::unique_ptr<std::ofstream> ofs;

    mutable std::mutex fileMutex;
};


class LocalFileHolderFactory : public FileHolderFactory
{
public:

    std::shared_ptr<FileHolder> makeFileHolder(const std::string& filename)
    {
        std::shared_ptr<LocalFileHolder> holder(new LocalFileHolder(filename));
        return std::static_pointer_cast<FileHolder>(holder);
    }

};

} //Utils
} //STI

#endif
