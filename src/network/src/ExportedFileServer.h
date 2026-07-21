#ifndef STI_NETWORK_EXPORTEDFILESERVER_H
#define STI_NETWORK_EXPORTEDFILESERVER_H

#include <sti/utils/FileServer.h>

#include "ServantHolder.h"
#include "TFileServer_i.h"
#include "generated/deviceNet.h"

#include <memory>

namespace STI
{
namespace Network
{

class ExportedFileServer
{
public:
    explicit ExportedFileServer(const std::shared_ptr<STI::Utils::FileServer>& fileServer);

    STI::TNetwork::TFileServer_var getTFileServerRef() const;

private:
    std::shared_ptr<STI::Utils::FileServer> fileServer;
    ServantHolder<STI::TNetwork::TFileServer_i, STI::TNetwork::TFileServer> servantHolder;
};

} //Network
} //STI

#endif
