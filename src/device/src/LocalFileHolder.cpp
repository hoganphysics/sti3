#include <sti/utils/LocalFileHolder.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/BinaryData.h>

#include "VirtualFileHolder.h"

#include <openssl/md5.h> 

#include <string>
#include <fstream>
#include <sstream> 
#include <iomanip> 
#include <filesystem>


using STI::Utils::FileHolder;
using STI::Utils::FileID;
using STI::Utils::LocalFileHolder;
using STI::Utils::FileTransferType;
using STI::Utils::LocalFileHolderFactory;
using STI::Utils::VirtualFileHolder;
using STI::Utils::BinaryData;


std::string digestToString(unsigned char (&digest)[MD5_DIGEST_LENGTH])
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0'); 

    for (unsigned char uc : digest) 
    ss << std::setw(2) << (int)uc; 
    
    return ss.str();
}


LocalFileHolder::LocalFileHolder(const std::string& originID, const std::string& path, const std::string& filename)
{
    fileID.persistenceLocation = originID;
    fileID.origin = originID;
    fileID.path = path;
    fileID.filename = filename;
}

LocalFileHolder::LocalFileHolder(const std::string& originID, const FileID& id)
: LocalFileHolder(originID, id.path, id.filename)
{
    fileID.origin = id.origin;  //keep origin
}

LocalFileHolder::~LocalFileHolder()
{
}

bool LocalFileHolder::operator==(const LocalFileHolder& other) const
{
    return other.getID() == getID();
}

bool LocalFileHolder::operator!=(const LocalFileHolder& other) const
{
    return !((*this) == other);
}

FileID LocalFileHolder::getID() const
{
    return fileID;
}

std::string LocalFileHolder::getFilename() const
{
    return fileID.getFullFilename();
}

unsigned LocalFileHolder::getFileSize() const
{
    std::filesystem::path filepath(getFilename());
    return std::filesystem::file_size(filepath);
}

bool LocalFileHolder::exists() const
{
    return std::filesystem::exists(getFilename());
}

unsigned LocalFileHolder::maxBufferSize() const
{
    return 16384;
}


bool LocalFileHolder::transferFile(const std::shared_ptr<FileHolder>& destination)
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    if (destination == 0) return false;

    std::shared_ptr<std::istream> ifs;
    if (!getistream(ifs)) return false;

    //get file size
    unsigned filesize = static_cast<unsigned>(ifs->tellg());
    ifs->seekg(0, std::ios::beg);   //move to beginning

    //Can only transfer to an new destination file
    if (destination->exists()) return false;

    //open destination file for writing
    if (!destination->openFile()) return false;

    //buffer
    unsigned maxBufferSize = destination->maxBufferSize();
    unsigned bufferSize;

    if (filesize < maxBufferSize) {
        bufferSize = filesize;
    }
    else {
        bufferSize = maxBufferSize;
    }
    char* buffer = new char[bufferSize];

    //compute md5 on local file while reading
    MD5_CTX md5Context; 
    MD5_Init(&md5Context); 

    unsigned amountRead = 0;
    unsigned nextReadSize = bufferSize;

    while (nextReadSize > 0 && ifs->read(buffer, nextReadSize))
    {
        amountRead += nextReadSize;
        nextReadSize = std::min(bufferSize, filesize - amountRead);

        destination->write(buffer, static_cast<unsigned>(ifs->gcount()));

        MD5_Update(&md5Context, buffer, ifs->gcount());
    }

    delete[] buffer;

    unsigned char digest[MD5_DIGEST_LENGTH];
    int res = MD5_Final(digest, &md5Context);

    if (res == 0) return false;     // hash failed 

    std::string hash = digestToString(digest);
    md5hash.set(hash);

    destination->closeFile();

    //copy fileID data
    auto remoteFileID = destination->getID();
    fileID.origin = remoteFileID.origin;
    fileID.creationTime = remoteFileID.creationTime;

    return md5hash == destination->md5Checksum();   //compare checksums
}

std::string LocalFileHolder::md5Checksum()
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    if (md5hash.isCached()) return md5hash.get();

    std::string hash;

    std::ifstream ifs(getFilename(), std::ifstream::binary);
    if (!ifs.is_open()) return "";

    LocalFileHolder::makeMD5hash(ifs, hash, 16384);
    ifs.close(); 
    
    md5hash.set(hash);

    return md5hash.get();
}


bool LocalFileHolder::openFile()
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    if (ofs != 0 && ofs->is_open()) return true;

    std::filesystem::path filepath(fileID.path);

    if (!std::filesystem::exists(filepath)) {
        std::filesystem::create_directories(filepath);
    }

    ofs = std::make_unique<std::ofstream>(getFilename(), std::ifstream::binary);

    return ofs != 0 && ofs->is_open();
}

std::ostream* LocalFileHolder::getostream()
{
    return ofs.get();
}

bool LocalFileHolder::getistream(std::shared_ptr<std::istream>& istream)
{
    auto ifs = std::make_shared<std::ifstream>(getFilename(), std::ifstream::binary|std::ios::ate);
    // std::ifstream ifs(getFilename(), std::ifstream::binary|std::ios::ate);
    if (!ifs->is_open()) return false;
    istream = ifs;
    return true;
}

// bool LocalFileHolder::openFileType(FileTransferType type, unsigned offset, unsigned& maxBufferSize)
// {
//     std::unique_lock<std::mutex> writeLock(fileMutex);

//     if (ofs != 0 && ofs->is_open()) return true;

//     if (type == FileTransferType::Binary) {
//         ofs = std::make_unique<std::ofstream>(getFilename(), std::ifstream::binary);
//     }
//     else if (type == FileTransferType::String) {
//         ofs = std::make_unique<std::ofstream>(getFilename());
//     }
    
//     ofs->seekp(offset);

//     maxBufferSize = maxBufferSize();

//     return ofs != 0 && ofs->is_open();
// }

bool LocalFileHolder::write(const char* buffer, unsigned length)
{
    if (getostream() != 0) {
        getostream()->write(buffer, length);
        return true;
    }
    return false;
}

bool LocalFileHolder::write(const std::shared_ptr<BinaryData>& data)
{
    if (data == 0) return false;

    char* cData;
    data->getBytes(cData);

    return write(cData, data->bytes());
}

void LocalFileHolder::closeFile()
{
    if (ofs != 0) {
        ofs->close();        
    }
}


bool LocalFileHolder::makeMD5hash(std::istream& ifs, std::string& md5string, unsigned bufferSize) 
{
    char* buffer = new char[bufferSize];
    
    MD5_CTX md5Context; 
    MD5_Init(&md5Context); 
    
    while (ifs.good())  
    { 
        ifs.read(buffer, bufferSize); 
    
        MD5_Update(&md5Context, buffer, ifs.gcount());
    } 
    
    delete[] buffer;


    unsigned char digest[MD5_DIGEST_LENGTH];
    int res = MD5_Final(digest, &md5Context); 
    
    if (res == 0) // hash failed 
        return false;   // or raise an exception 
    
    md5string = digestToString(digest);
    
    return true;
}


//LocalFileHolderFactory

LocalFileHolderFactory::LocalFileHolderFactory(const std::string& originID) 
: originID(originID)
{
}

std::shared_ptr<FileHolder> LocalFileHolderFactory::makeFileHolder(const std::string& path, const std::string& filename)
{
    std::shared_ptr<LocalFileHolder> holder(new LocalFileHolder(originID, path, filename));
    return std::static_pointer_cast<FileHolder>(holder);
}

std::shared_ptr<FileHolder> LocalFileHolderFactory::makeVirtualFileHolder(const FileID& fileID)
{
    std::filesystem::path newPath = originID;   //prepend orignID before path of virtual files
    newPath /= fileID.path;
    STI::Utils::FileID newFileID = fileID;
    newFileID.path = newPath.string();

    std::shared_ptr<VirtualFileHolder> holder(new VirtualFileHolder(originID, newFileID));
    return std::static_pointer_cast<FileHolder>(holder);
}

