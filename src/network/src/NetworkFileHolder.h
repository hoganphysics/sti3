#ifndef STI_ENGINE_NETWORKFILEHOLDER_H
#define STI_ENGINE_NETWORKFILEHOLDER_H

#include "LocalFileHolder.h"

#include "TFileHolderRefInterface.h"

#include "TFileHolder_i.h"
#include "deviceNet.h"

#include <vector>
#include <memory>


namespace STI
{
namespace Network
{


class NetworkFileHolder : public STI::Utils::LocalFileHolder,
                          public STI::Network::TFileHolderRefInterface	//mixin
{
public:

    NetworkFileHolder(const std::string& filename)
    : STI::Utils::LocalFileHolder(filename), fileHolderServant(this)
    {
    }

    ~NetworkFileHolder()
    {
    }

    // std::string getFilename() const
    // {
    //     if (localFileHolder != 0) {
    //         return localFileHolder->getFilename();
    //     }
    //     return "";
    // }

    // bool exists() const
    // {
    //     return localFileHolder != 0 && localFileHolder->exists();
    // }

    // bool transferFile(const std::shared_ptr<FileHolder>& destination)
    // {
    //     return localFileHolder != 0 && localFileHolder->transferFile(destination);
    // }

    // unsigned maxBufferSize() const
    // {
    //     if (localFileHolder != 0) {
    //         return localFileHolder->maxBufferSize();
    //     }
    //     return 32*1000;
    // }

    // bool deleteFile()
    // {
    //     return localFileHolder != 0 && localFileHolder->deleteFile();
    // }

    // std::string md5Checksum()
    // {
    //     if (localFileHolder != 0) {
    //         return localFileHolder->md5Checksum();
    //     }
    //     return "";
    // }

    // bool write(const char* buffer, unsigned length)
    // {
    //     return localFileHolder != 0 && localFileHolder->write(buffer, length);
    // }

    // bool openFile()
    // {
    //     return localFileHolder != 0 && localFileHolder->openFile();
    // }

    // void closeFile()
    // {
    //     if (localFileHolder != 0) {
    //         localFileHolder->closeFile();
    //     }
    // }

private:

    bool getTFileHolderRef(STI::TNetwork::TFileHolder_var& tFileHolder)
    {
        tFileHolder = fileHolderServant._this();
        return !CORBA::is_nil(tFileHolder);
    }

    std::shared_ptr<STI::Utils::FileHolder> localFileHolder;
    STI::TNetwork::TFileHolder_i fileHolderServant;
};


class NetworkFileHolderFactory : public STI::Utils::FileHolderFactory
{
public:

    std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& filename)
    {
        std::shared_ptr<NetworkFileHolder> holder(new NetworkFileHolder(filename));
        return std::static_pointer_cast<STI::Utils::FileHolder>(holder);
    }

};


} //Network
} //STI

#endif
