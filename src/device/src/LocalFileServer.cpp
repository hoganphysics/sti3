
#include "LocalFileServer.h"

#include <sti/utils/VirtualFileHolder.h>

#include <filesystem>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

using STI::Utils::LocalFileServer;
using STI::Utils::FileTransferType;
using STI::Utils::FileID;
using STI::Utils::VirtualFileHolder;

namespace {

bool writeLinesToDestination(
    std::istream& stream,
    const std::shared_ptr<STI::Utils::FileHolder>& destination,
    int offset,
    int lines)
{
    if (destination == nullptr || lines <= 0) {
        return false;
    }

    if (!destination->openFile()) {
        return false;
    }

    bool wroteAny = false;
    std::string line;
    auto writeLine = [&destination](const std::string& lineText) {
        return destination->write(lineText.data(), static_cast<unsigned>(lineText.size()))
            && destination->write("\n", 1);
    };

    if (offset >= 0) {
        int currentLine = 0;
        while (currentLine < offset && std::getline(stream, line)) {
            ++currentLine;
        }

        int linesRemaining = lines;
        while (linesRemaining > 0 && std::getline(stream, line)) {
            if (!writeLine(line)) {
                destination->closeFile();
                return false;
            }
            wroteAny = true;
            --linesRemaining;
        }
    }
    else {
        const std::size_t tailWindow = static_cast<std::size_t>(-offset) + static_cast<std::size_t>(lines) - 1;
        std::deque<std::string> tailLines;

        while (std::getline(stream, line)) {
            tailLines.push_back(line);
            if (tailLines.size() > tailWindow) {
                tailLines.pop_front();
            }
        }

        const std::size_t startIndex = (tailLines.size() > static_cast<std::size_t>(-offset))
            ? tailLines.size() - static_cast<std::size_t>(-offset)
            : 0;

        for (std::size_t i = startIndex; i < tailLines.size(); ++i) {
            if (i >= startIndex + static_cast<std::size_t>(lines)) {
                break;
            }
            if (!writeLine(tailLines[i])) {
                destination->closeFile();
                return false;
            }
            wroteAny = true;
        }
    }

    destination->closeFile();
    return wroteAny || lines > 0;
}

} // namespace


LocalFileServer::LocalFileServer(const STI::Device::DeviceID& localID)
: localID(localID), localFileHolderFactory(localID.getID())
{

}

LocalFileServer::~LocalFileServer()
{
}

bool LocalFileServer::addFile(const std::shared_ptr<STI::Utils::FileHolder>& file)
{
    if (file == nullptr) return false;

    return registeredFiles.add(file->getID(), file);
}

bool LocalFileServer::findFile(const FileID& fileID)
{
    std::shared_ptr<STI::Utils::FileHolder> registeredFile;
    if (registeredFiles.get(fileID, registeredFile) && registeredFile != nullptr) {
        return registeredFile->exists();
    }

    if (fileID.persistenceLocation != localID.getID()) return false;

    fs::path filePath = fileID.path;
    filePath /= fileID.filename;

    return fs::exists(filePath);
}

int LocalFileServer::getFileSize(const FileID& fileID)
{
    std::shared_ptr<STI::Utils::FileHolder> registeredFile;
    if (registeredFiles.get(fileID, registeredFile) && registeredFile != nullptr) {
        return registeredFile->exists() ? static_cast<int>(registeredFile->getFileSize()) : 0;
    }

    if (!findFile(fileID)) return false;

    fs::path filePath = fileID.path;
    filePath /= fileID.filename;

    return fs::file_size(filePath);
}

/// Transfer source file to (remote) destination
bool LocalFileServer::transferFile(const FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, FileTransferType type)
{
    if (destination == 0) return false;

    std::shared_ptr<STI::Utils::FileHolder> registeredFile;
    if (registeredFiles.get(source, registeredFile) && registeredFile != nullptr) {
        return registeredFile->exists() && registeredFile->transferFile(destination);
    }

    if (!findFile(source)) return false;    //could not find source file on local machine

    auto localFile = localFileHolderFactory.makeFileHolder(source.path, source.filename);

    return localFile->transferFile(destination);
}

bool LocalFileServer::transferFilePartial(const FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, int offset, int lines)
{
    if (destination == nullptr) return false;

    std::shared_ptr<STI::Utils::FileHolder> registeredFile;
    if (registeredFiles.get(source, registeredFile) && registeredFile != nullptr) {
        auto virtualFile = std::dynamic_pointer_cast<VirtualFileHolder>(registeredFile);
        if (virtualFile == nullptr || !virtualFile->exists()) {
            return false;
        }

        std::istringstream stream(virtualFile->getBytes());
        return writeLinesToDestination(stream, destination, offset, lines);
    }

    if (!findFile(source)) return false;

    fs::path filePath = source.path;
    filePath /= source.filename;

    std::ifstream stream(filePath);
    if (!stream.is_open()) {
        return false;
    }

    return writeLinesToDestination(stream, destination, offset, lines);
}

bool LocalFileServer::deleteFile(const FileID& fileID)
{
    std::shared_ptr<STI::Utils::FileHolder> registeredFile;
    if (registeredFiles.get(fileID, registeredFile) && registeredFile != nullptr) {
        return registeredFiles.remove(fileID);
    }

    if (!findFile(fileID)) return false;

    fs::path filePath = fileID.path;
    filePath /= fileID.filename;

    return std::filesystem::remove(filePath);
}
