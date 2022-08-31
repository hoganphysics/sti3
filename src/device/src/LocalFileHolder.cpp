

#include <sti/utils/LocalFileHolder.h>

#include <string>
#include <fstream>

#include <openssl/md5.h> 
#include <sstream> 
#include <iomanip> 

#include <filesystem>

#include "CerealArchives.h"
#include <cereal/types/string.hpp>
#include <cereal/types/polymorphic.hpp>

using STI::Utils::FileHolder;
using STI::Utils::LocalFileHolder;


//Serialization
CEREAL_REGISTER_TYPE(LocalFileHolder);
CEREAL_REGISTER_POLYMORPHIC_RELATION(FileHolder, LocalFileHolder)




std::string digestToString(unsigned char (&digest)[MD5_DIGEST_LENGTH])
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0'); 

    for (unsigned char uc : digest) 
    ss << std::setw(2) << (int)uc; 
    
    return ss.str();
}


LocalFileHolder::LocalFileHolder(const std::string& filename)
: filename(filename), hashed(false)
{
}

LocalFileHolder::LocalFileHolder()
: hashed(false)
{
}

LocalFileHolder::~LocalFileHolder()
{
}


std::string LocalFileHolder::getFilename() const
{
    return filename;
}


bool LocalFileHolder::exists() const
{
    return std::filesystem::exists(filename);
}


unsigned LocalFileHolder::maxBufferSize() const
{
    return 16384;
}


bool LocalFileHolder::transferFile(const std::shared_ptr<FileHolder>& destination)
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    if (destination == 0) return false;

    std::ifstream ifs(filename, std::ifstream::binary|std::ios::ate);
    if (!ifs.is_open()) return false;

    //get file size
    unsigned filesize = static_cast<unsigned>(ifs.tellg());
    ifs.seekg(0, std::ios::beg);   //move to beginning

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

    while (nextReadSize > 0 && ifs.read(buffer, nextReadSize))
    {
        amountRead += nextReadSize;
        nextReadSize = std::min(bufferSize, filesize - amountRead);

        destination->write(buffer, static_cast<unsigned>(ifs.gcount()));

        MD5_Update(&md5Context, buffer, ifs.gcount());
    }

    delete[] buffer;

    unsigned char digest[MD5_DIGEST_LENGTH];
    int res = MD5_Final(digest, &md5Context);

    if (res == 0) return false;     // hash failed 

    md5hash = digestToString(digest);
    hashed = true;

    ifs.close(); 
    destination->closeFile();

    return md5hash == destination->md5Checksum();   //compare checksums
}

bool LocalFileHolder::deleteFile()
{
    std::filesystem::path filepath(filename);

    return std::filesystem::remove(filepath);
}

std::string LocalFileHolder::md5Checksum()
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    if (!hashed) {

        hashed = LocalFileHolder::makeMD5hash(filename, md5hash, 16384);
    }

    return md5hash;
}


bool LocalFileHolder::openFile()
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    if (ofs != 0 && ofs->is_open()) return true;

    ofs = std::make_unique<std::ofstream>(filename, std::ifstream::binary);

    return ofs != 0 && ofs->is_open();
}

bool LocalFileHolder::write(const char* buffer, unsigned length)
{
    if (ofs != 0) {
        ofs->write(buffer, length);
        return true;
    }
    return false;
}

void LocalFileHolder::closeFile()
{
    if (ofs != 0) {
        ofs->close();        
    }
}


bool LocalFileHolder::makeMD5hash(const std::string& fname, std::string& md5string, unsigned bufferSize) 
{
    char* buffer = new char[bufferSize];

    std::ifstream ifs(fname, std::ifstream::binary);

    if (! ifs.is_open()) return false;
    
    MD5_CTX md5Context; 
    MD5_Init(&md5Context); 
    
    while (ifs.good())  
    { 
        ifs.read(buffer, bufferSize); 
    
        MD5_Update(&md5Context, buffer, ifs.gcount());
    } 
    
    delete[] buffer;

    ifs.close(); 

    unsigned char digest[MD5_DIGEST_LENGTH];
    int res = MD5_Final(digest, &md5Context); 
    
    if (res == 0) // hash failed 
        return false;   // or raise an exception 
    
    md5string = digestToString(digest);
    
    return true;
}

template<class Archive>
void LocalFileHolder::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("filename", filename), 
		cereal::make_nvp("md5hash", md5hash), 
		cereal::make_nvp("hashed", hashed)
		);
}

template void LocalFileHolder::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void LocalFileHolder::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

