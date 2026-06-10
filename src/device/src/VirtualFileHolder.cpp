
#include <sti/utils/VirtualFileHolder.h>


using STI::Utils::VirtualFileHolder;
using STI::Utils::FileID;


VirtualFileHolder::VirtualFileHolder(const std::string& originID, const FileID& fileID)
: LocalFileHolder(originID, fileID)
{
    data = std::make_shared<std::stringstream>();
    hasFile = false;
}

VirtualFileHolder::~VirtualFileHolder()
{
}

std::ostream* VirtualFileHolder::getostream()
{
    return data.get();
}

bool VirtualFileHolder::getistream(std::shared_ptr<std::istream>& istream)
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    if (data == 0) return false;

    // data->seekg(std::ios_base::end);
    data->seekg(0, std::ios::end);

    istream = data;
    return true;
}

unsigned VirtualFileHolder::getFileSize() const
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    if (data == 0) return 0;
    return static_cast<unsigned>(data->str().size());
}

bool VirtualFileHolder::exists() const
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    return hasFile;
}

std::string VirtualFileHolder::md5Checksum()
{
    std::string payload = getBytes();
    std::istringstream stream(payload);
    std::string hash;

    if (!LocalFileHolder::makeMD5hash(stream, hash, maxBufferSize())) {
        return "";
    }
    return hash;
}

std::string VirtualFileHolder::getBytes() const
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    if (data == 0) return "";
    return data->str();
}

bool VirtualFileHolder::openFile()
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    data = std::make_shared<std::stringstream>();
    data->clear();
    data->str("");
    hasFile = false;
    return true;
}

void VirtualFileHolder::closeFile()
{
    std::unique_lock<std::mutex> writeLock(fileMutex);

    hasFile = true;
    // std::cout << "File: " << getID().filename << std::endl;

    // std::cout << "File: " << data->str() << std::endl;
}

VirtualFileHolder& VirtualFileHolder::operator<<(manip1 fp)
{
    (*data) << fp;
    return *this;
}

VirtualFileHolder& VirtualFileHolder::operator<<(manip2 fp)
{
    (*data) << fp;
    return *this;
}

VirtualFileHolder& VirtualFileHolder::operator<<(manip3 fp)
{
    (*data) << fp;
    return *this;
}
