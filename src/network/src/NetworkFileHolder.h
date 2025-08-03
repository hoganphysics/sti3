#ifndef STI_ENGINE_NETWORKFILEHOLDER_H
#define STI_ENGINE_NETWORKFILEHOLDER_H

#include <sti/utils/LocalFileHolder.h>

#include "TFileHolderRefInterface.h"
#include <sti/utils/VirtualFileHolder.h>
#include "TFileHolder_i.h"
#include "generated/deviceNet.h"

#include <vector>
#include <memory>


namespace STI
{
namespace Network
{


class NetworkFileHolder : public STI::Utils::FileHolder,
                          public STI::Network::TFileHolderRefInterface	//mixin
{
public:

    NetworkFileHolder(const STI::Utils::FileID& fileID, const std::shared_ptr<STI::Utils::FileHolder>& fileHolder);
    virtual ~NetworkFileHolder();

    static bool isNetworkFileHolder(const std::shared_ptr<STI::Utils::FileHolder>& fileHolder)
    {
        auto wrapper = std::dynamic_pointer_cast<NetworkFileHolder>(fileHolder);
        return (wrapper != 0);
    }

    STI::Utils::FileID getID() const;
    std::string getFilename() const;

    unsigned getFileSize() const;
    bool exists() const;

    bool transferFile(const std::shared_ptr<FileHolder>& destination);
    unsigned maxBufferSize() const;
    std::string md5Checksum();
    
    bool write(const char* buffer, unsigned length);
	// virtual bool writeString(const std::string& buffer) = 0;

    bool openFile();    //before writing
    void closeFile();


private:

    bool getTFileHolderRef(STI::TNetwork::TFileHolder_var& tFileHolder);

    std::shared_ptr<STI::Utils::FileHolder> localFileHolder;
    STI::TNetwork::TFileHolder_i fileHolderServant;
};


class NetworkFileHolderFactory : public STI::Utils::FileHolderFactory
{
public:
    
    NetworkFileHolderFactory(const std::string& originID);

    std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& path, const std::string& filename);

    std::shared_ptr<STI::Utils::FileHolder> makeVirtualFileHolder(const STI::Utils::FileID& fileID);

    std::string originID;
};


} //Network
} //STI

#endif
