#include "NetworkFileServer.h"
#include "LocalFileServer.h"

using STI::Network::NetworkFileServer;
using STI::Network::NetworkVirtualFileServer;


NetworkFileServer::NetworkFileServer(const STI::Device::DeviceID& localID)
: STI::Utils::LocalFileServer(localID), fileServerServant(this)
{
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
}

NetworkVirtualFileServer::~NetworkVirtualFileServer()
{
}

bool NetworkVirtualFileServer::getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer)
{
    tFileServer = fileServerServant._this();
    return !CORBA::is_nil(tFileServer);
}

