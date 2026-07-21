#include "ExportedFileServer.h"

#include <stdexcept>

using STI::Network::ExportedFileServer;

ExportedFileServer::ExportedFileServer(const std::shared_ptr<STI::Utils::FileServer>& fileServer)
    : fileServer(fileServer)
{
    if (this->fileServer == nullptr) {
        throw std::invalid_argument("ExportedFileServer requires a FileServer");
    }

    // emplace constructs the TFileServer_i servant. The servant borrows this raw
    // pointer, while the shared_ptr member above keeps the FileServer alive.
    servantHolder.emplace(this->fileServer.get());
}

STI::TNetwork::TFileServer_var ExportedFileServer::getTFileServerRef() const
{
    return servantHolder.getRefVar();
}
