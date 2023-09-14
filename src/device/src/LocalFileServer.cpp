
#include "LocalFileServer.h"

#include <filesystem>

namespace fs = std::filesystem;

using STI::Utils::LocalFileServer;
using STI::Utils::FileTransferType;
using STI::Utils::FileID;


LocalFileServer::LocalFileServer(const STI::Device::DeviceID& localID)
: localID(localID), localFileHolderFactory(localID.getID())
{

}

LocalFileServer::~LocalFileServer()
{
}

bool LocalFileServer::findFile(const FileID& fileID)
{
    if (fileID.persistenceLocation != localID.getID()) return false;

    fs::path filePath = fileID.path;
    filePath /= fileID.filename;

    return fs::exists(filePath);
}

int LocalFileServer::getFileSize(const FileID& fileID)
{
    if (!findFile(fileID)) return false;

    fs::path filePath = fileID.path;
    filePath /= fileID.filename;

    return fs::file_size(filePath);
}

/// Transfer source file to (remote) destination
bool LocalFileServer::transferFile(const FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, FileTransferType type)
{
    if (destination == 0) return false;
    if (!findFile(source)) return false;    //could not find source file on local machine

    auto localFile = localFileHolderFactory.makeFileHolder(source.path, source.filename);

    return localFile->transferFile(destination);
}

bool LocalFileServer::transferFilePartial(const FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, int offset, int lines)
{
    return false;
}

bool LocalFileServer::deleteFile(const FileID& fileID)
{
    if (!findFile(fileID)) return false;

    fs::path filePath = fileID.path;
    filePath /= fileID.filename;

    return std::filesystem::remove(filePath);
}



