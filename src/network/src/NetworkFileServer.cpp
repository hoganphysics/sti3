
#include "NetworkFileServer.h"

#include "LocalFileServer.h"

using STI::Network::NetworkFileServer;


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
