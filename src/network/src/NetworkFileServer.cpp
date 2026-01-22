#include "NetworkFileServer.h"
#include "LocalFileServer.h"
#include "ORBManager.h"

using STI::Network::NetworkFileServer;
using STI::Network::NetworkVirtualFileServer;


NetworkFileServer::NetworkFileServer(const STI::Device::DeviceID& localID)
: STI::Utils::LocalFileServer(localID), fileServerServant(this)
{
    STI::Network::ORBManager::ORBManager::activateServant(fileServerServant);
}

NetworkFileServer::~NetworkFileServer()
{
}

bool NetworkFileServer::getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer)
{
    tFileServer = fileServerServant._this();
    return !CORBA::is_nil(tFileServer);
}


////////////////////////////


NetworkVirtualFileServer::NetworkVirtualFileServer()
: STI::Utils::VirtualFileServer(), fileServerServant(this)
{
    STI::Network::ORBManager::ORBManager::activateServant(fileServerServant);
}

NetworkVirtualFileServer::~NetworkVirtualFileServer()
{
}

bool NetworkVirtualFileServer::getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer)
{
    tFileServer = fileServerServant._this();
    return !CORBA::is_nil(tFileServer);
}

