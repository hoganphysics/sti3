#include "CompositeFileServer.h"

#include <utility>

namespace STI
{
namespace Device
{

CompositeFileServer::CompositeFileServer(std::vector<std::shared_ptr<STI::Utils::FileServer>> servers)
    : servers(std::move(servers))
{
}

bool CompositeFileServer::addFile(const std::shared_ptr<STI::Utils::FileHolder>& file)
{
    return !servers.empty() && servers.front() != nullptr && servers.front()->addFile(file);
}

bool CompositeFileServer::findFile(const STI::Utils::FileID& fileID)
{
    return findServer(fileID) != nullptr;
}

int CompositeFileServer::getFileSize(const STI::Utils::FileID& fileID)
{
    auto server = findServer(fileID);
    return server == nullptr ? 0 : server->getFileSize(fileID);
}

bool CompositeFileServer::transferFile(const STI::Utils::FileID& source,
    const std::shared_ptr<STI::Utils::FileHolder>& destination,
    STI::Utils::FileTransferType type)
{
    auto server = findServer(source);
    return server != nullptr && server->transferFile(source, destination, type);
}

bool CompositeFileServer::transferFilePartial(const STI::Utils::FileID& source,
    const std::shared_ptr<STI::Utils::FileHolder>& destination,
    int offset,
    int lines)
{
    auto server = findServer(source);
    return server != nullptr && server->transferFilePartial(source, destination, offset, lines);
}

bool CompositeFileServer::deleteFile(const STI::Utils::FileID& fileID)
{
    bool deleted = false;
    for (auto& server : servers) {
        if (server != nullptr && server->findFile(fileID)) {
            deleted = server->deleteFile(fileID) || deleted;
        }
    }
    return deleted;
}

std::shared_ptr<STI::Utils::FileServer> CompositeFileServer::findServer(const STI::Utils::FileID& fileID)
{
    for (auto& server : servers) {
        if (server != nullptr && server->findFile(fileID)) {
            return server;
        }
    }
    return nullptr;
}

std::shared_ptr<STI::Utils::FileServer> makeMeasurementSourceFileServer(
    const std::shared_ptr<STI::Utils::FileServer>& measurementFileServer,
    const std::shared_ptr<STI::Utils::FileServer>& deviceFileServer)
{
    if (measurementFileServer == nullptr) {
        return deviceFileServer;
    }
    if (deviceFileServer == nullptr || measurementFileServer == deviceFileServer) {
        return measurementFileServer;
    }

    return std::make_shared<CompositeFileServer>(
        std::vector<std::shared_ptr<STI::Utils::FileServer>>{ measurementFileServer, deviceFileServer });
}

} //Device
} //STI
