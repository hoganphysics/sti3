#include "NetworkFileServer.h"
#include "LocalFileServer.h"
#include "ORBManager.h"

using STI::Network::NetworkFileServer;
using STI::Network::NetworkVirtualFileServer;


NetworkFileServer::NetworkFileServer(const STI::Device::DeviceID& localID)
: STI::Utils::LocalFileServer(localID), fileServerServantHolder(new STI::TNetwork::TFileServer_i(this))
{
}

NetworkFileServer::~NetworkFileServer()
{
}

bool NetworkFileServer::getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer)
{
    tFileServer = fileServerServantHolder.getRefVar();
    return !CORBA::is_nil(tFileServer);
}


////////////////////////////


NetworkVirtualFileServer::NetworkVirtualFileServer()
: STI::Utils::VirtualFileServer(), fileServerServantHolder(new STI::TNetwork::TFileServer_i(this))
{
}

NetworkVirtualFileServer::~NetworkVirtualFileServer()
{
}

bool NetworkVirtualFileServer::getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer)
{
    tFileServer = fileServerServantHolder.getRefVar();
    return !CORBA::is_nil(tFileServer);
}

