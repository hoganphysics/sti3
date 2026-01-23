#ifndef STI_ENGINE_NETWORKFILESERVER_H
#define STI_ENGINE_NETWORKFILESERVER_H

#include <sti/utils/VirtualFileServer.h>

#include "LocalFileServer.h"
#include "TFileServerRefInterface.h"
#include "TFileServer_i.h"
#include "generated/deviceNet.h"
#include "ServantHolder.h"

#include <vector>
#include <memory>


namespace STI
{
namespace Network
{


class NetworkFileServer : public STI::Utils::LocalFileServer,
                          public STI::Network::TFileServerRefInterface	//mixin
{
public:

    NetworkFileServer(const STI::Device::DeviceID& localID);
    ~NetworkFileServer();

    static bool isNetworkFileServer(const std::shared_ptr<STI::Utils::FileServer>& fileServer)
    {
        auto wrapper = std::dynamic_pointer_cast<TFileServerRefInterface>(fileServer);
        return (wrapper != 0);
    }

private:

    bool getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer);

    // STI::TNetwork::TFileServer_i fileServerServant;
    ServantHolder<STI::TNetwork::TFileServer_i, STI::TNetwork::TFileServer> fileServerServantHolder;
};


class NetworkVirtualFileServer : public STI::Utils::VirtualFileServer,
                                 public STI::Network::TFileServerRefInterface	//mixin
{
public:

    NetworkVirtualFileServer();
    ~NetworkVirtualFileServer();

private:

    bool getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer);
    // STI::TNetwork::TFileServer_i fileServerServant;
    ServantHolder<STI::TNetwork::TFileServer_i, STI::TNetwork::TFileServer> fileServerServantHolder;
};


class NetworkVirtualFileServerFactory : public STI::Utils::VirtualFileServerFactory
{
public:

	std::shared_ptr<STI::Utils::VirtualFileServer> makeVirtualFileServer()
	{
		auto fileServer = std::make_shared<NetworkVirtualFileServer>();
		return fileServer;
	}
};


} //Network
} //STI

#endif
