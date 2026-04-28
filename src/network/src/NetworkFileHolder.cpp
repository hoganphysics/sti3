#include "NetworkFileHolder.h"
#include "ORBManager.h"

#include <filesystem>

using STI::Network::NetworkFileHolder;
using STI::Utils::FileHolder;
using STI::Network::NetworkFileHolderFactory;


NetworkFileHolder::NetworkFileHolder(const STI::Utils::FileID& fileID, const std::shared_ptr<STI::Utils::FileHolder>& fileHolder)
: fileHolderServantHolder(new STI::TNetwork::TFileHolder_i(this))
{
    if (fileHolder != 0) {
        localFileHolder = fileHolder;
    }
    else {
        localFileHolder = std::make_shared<STI::Utils::LocalFileHolder>(fileID.origin, fileID.path, fileID.filename);
    }
}

NetworkFileHolder::~NetworkFileHolder()
{
}

bool NetworkFileHolder::getTFileHolderRef(STI::TNetwork::TFileHolder_var& tFileHolder)
{
    tFileHolder = fileHolderServantHolder.getRefVar();
    return !CORBA::is_nil(tFileHolder);
}


STI::Utils::FileID NetworkFileHolder::getID() const
{
    return localFileHolder->getID();
}

std::string NetworkFileHolder::getFilename() const
{
    return localFileHolder->getFilename();
}

unsigned NetworkFileHolder::getFileSize() const
{
    return localFileHolder->getFileSize();
}

bool NetworkFileHolder::exists() const
{
    return localFileHolder->exists();
}

bool NetworkFileHolder::transferFile(const std::shared_ptr<FileHolder>& destination)
{
    return localFileHolder->transferFile(destination);
}

unsigned NetworkFileHolder::maxBufferSize() const
{
    return localFileHolder->maxBufferSize();
}

std::string NetworkFileHolder::md5Checksum()
{
    return localFileHolder->md5Checksum();
}

bool NetworkFileHolder::write(const char* buffer, unsigned length)
{
    return localFileHolder->write(buffer, length);
}

bool NetworkFileHolder::openFile()
{
    return localFileHolder->openFile();
}

void NetworkFileHolder::closeFile()
{
    return localFileHolder->closeFile();
}



///NetworkFileHolderFactory


NetworkFileHolderFactory::NetworkFileHolderFactory(const std::string& originID) 
: originID(originID) 
{
}

std::shared_ptr<STI::Utils::FileHolder> NetworkFileHolderFactory::makeFileHolder(const std::string& path, const std::string& filename)
{
    auto localHolder = std::make_shared<STI::Utils::LocalFileHolder>(originID, path, filename);
    auto holder = std::make_shared<NetworkFileHolder>(localHolder->getID(), localHolder);
    return std::static_pointer_cast<STI::Utils::FileHolder>(holder);
}

std::shared_ptr<STI::Utils::FileHolder> NetworkFileHolderFactory::makeVirtualFileHolder(const STI::Utils::FileID& fileID)
{
    std::filesystem::path newPath = originID;   //prepend orignID before path of virtual files
    newPath /= fileID.path;
    STI::Utils::FileID newFileID = fileID;
    newFileID.path = newPath.string();
    
    auto virtualHolder = std::make_shared<STI::Utils::VirtualFileHolder>(originID, newFileID);
    auto holder = std::make_shared<NetworkFileHolder>(virtualHolder->getID(), virtualHolder);
    return std::static_pointer_cast<STI::Utils::FileHolder>(holder);
}

std::shared_ptr<STI::Utils::FileHolder> NetworkFileHolderFactory::makeVirtualFileHolder(
    const std::shared_ptr<STI::Utils::VirtualFileHolder>& backingHolder)
{
    if (backingHolder == 0) {
        return std::shared_ptr<STI::Utils::FileHolder>();
    }

    auto holder = std::make_shared<NetworkFileHolder>(backingHolder->getID(), backingHolder);
    return std::static_pointer_cast<STI::Utils::FileHolder>(holder);
}
