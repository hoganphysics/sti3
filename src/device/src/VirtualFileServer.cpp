
#include <sti/utils/VirtualFileServer.h>

using STI::Utils::VirtualFileServer;
using STI::Utils::FileHolder;
using STI::Utils::FileTransferType;
using STI::Utils::FileID;



VirtualFileServer::VirtualFileServer()
{
}

VirtualFileServer::~VirtualFileServer()
{
}

void VirtualFileServer::addFile(const std::shared_ptr<FileHolder>& file)
{
    if (file == 0) return;
    files.add(file->getID(), file);
}

bool VirtualFileServer::findFile(const FileID& fileID)
{
    return files.contains(fileID);
}

int VirtualFileServer::getFileSize(const FileID& fileID)
{
    std::shared_ptr<FileHolder> sourceFile;

    if (files.get(fileID, sourceFile) && sourceFile != 0) {
        return sourceFile->getFileSize();
    }
    return 0;
}

bool VirtualFileServer::transferFile(const FileID& sourceID, const std::shared_ptr<FileHolder>& destination, FileTransferType type)
{
    std::shared_ptr<FileHolder> sourceFile;

    if (files.get(sourceID, sourceFile) && sourceFile != 0) {
        return sourceFile->transferFile(destination);
    }
    return false;
}

bool VirtualFileServer::transferFilePartial(const FileID& source, const std::shared_ptr<FileHolder>& destination, int offset, int lines)
{
    return false;
}

bool VirtualFileServer::deleteFile(const FileID& fileID)
{
    return files.remove(fileID);
}


