#ifndef STI_ENGINE_NETWORKFILEHOLDER_H
#define STI_ENGINE_NETWORKFILEHOLDER_H

#include <sti/utils/LocalFileHolder.h>

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

    NetworkFileHolder(const std::string& filename);
    virtual ~NetworkFileHolder();

    static bool isNetworkFileHolder(const std::shared_ptr<STI::Utils::FileHolder>& fileHolder)
    {
        auto wrapper = std::dynamic_pointer_cast<NetworkFileHolder>(fileHolder);
        return (wrapper != 0);
    }

private:

    bool getTFileHolderRef(STI::TNetwork::TFileHolder_var& tFileHolder);

    std::shared_ptr<STI::Utils::FileHolder> localFileHolder;
    STI::TNetwork::TFileHolder_i fileHolderServant;
};


class NetworkFileHolderFactory : public STI::Utils::FileHolderFactory
{
public:

    std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& filename)
    {
        // std::shared_ptr<NetworkFileHolder> holder(new NetworkFileHolder(filename));
        auto holder = std::make_shared<NetworkFileHolder>(filename);
        return std::static_pointer_cast<STI::Utils::FileHolder>(holder);
    }

};


} //Network
} //STI

#endif
