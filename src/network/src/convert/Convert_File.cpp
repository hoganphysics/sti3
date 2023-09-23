#include "Convert_File.h"
#include "Convert_EventEngine.h"

#include "NetworkFileServer.h"
#include "RemoteFileServer.h"


using STI::Utils::FileID;
using STI::TNetwork::TFileID;
using STI::Utils::FileTransferType;
using STI::TNetwork::TFileTransferType;
using STI::Utils::FileServer;
using STI::TNetwork::TFileServer_var;
using STI::Network::RemoteFileServer;
using STI::Network::NetworkFileServer;


//FileID
template<>
FileID STI::Network::convert<TFileID, FileID>(const TFileID& tFileID)
{
    FileID fileID;
    convert<TFileID, FileID>(tFileID, fileID);
    return fileID;
}

template<>
TFileID STI::Network::convert<FileID, TFileID>(const FileID& fileID)
{
    TFileID tFileID;
    convert<FileID, TFileID>(fileID, tFileID);
    return tFileID;
}


template<>
bool STI::Network::convert<TFileID, FileID>(const TFileID& tFileID, FileID& fileID)
{
    fileID.filename = convert<CORBA::String_member, std::string>(tFileID.filename);
    fileID.path = convert<CORBA::String_member, std::string>(tFileID.path);
    fileID.origin = convert<CORBA::String_member, std::string>(tFileID.origin);
    fileID.persistenceLocation = convert<CORBA::String_member, std::string>(tFileID.persistenceLocation);
    fileID.creationTime = convert<STI::TNetwork::TTimeStamp, STI::Utils::TimeStamp>(tFileID.creationTime);
    return true;
}

template<>
bool STI::Network::convert<FileID, TFileID>(const FileID& fileID, TFileID& tFileID)
{
    tFileID.filename = convert<std::string, CORBA::String_member>(fileID.filename);
    tFileID.path = convert<std::string, CORBA::String_member>(fileID.path);
    tFileID.origin = convert<std::string, CORBA::String_member>(fileID.origin);
    tFileID.persistenceLocation = convert<std::string, CORBA::String_member>(fileID.persistenceLocation);
    tFileID.creationTime = convert<STI::Utils::TimeStamp, STI::TNetwork::TTimeStamp>(fileID.creationTime);
    return true;
}


//FileTransferType
template<>
FileTransferType STI::Network::convert<TFileTransferType, FileTransferType>(const TFileTransferType& tType)
{
    FileTransferType type;

    switch (tType) 
    {
    case TFileTransferType::FileTransferBinary:
        type = FileTransferType::Binary;
        break;
    case TFileTransferType::FileTransferString:
        type = FileTransferType::String;
        break;
    default:
        type = FileTransferType::Binary;
        break;
    }
    return type;
}

template<>
TFileTransferType STI::Network::convert<FileTransferType, TFileTransferType>(const FileTransferType& type)
{
    TFileTransferType tType;

    switch (type) 
    {
    case FileTransferType::Binary:
        tType = TFileTransferType::FileTransferBinary;
        break;
    case FileTransferType::String:
        tType = TFileTransferType::FileTransferString;
        break;
    default:
        tType = TFileTransferType::FileTransferBinary;
        break;
    }
    return tType;
}


//FileServer
template<>
bool STI::Network::convert<TFileServer_var, std::shared_ptr<FileServer>>(
        const TFileServer_var& tFileServer, std::shared_ptr<FileServer>& fileServer)
{
    if (!CORBA::is_nil(tFileServer)) {
        fileServer = std::make_shared<RemoteFileServer>(tFileServer);
        return (fileServer != 0);
    }
    return false;
}

template<>
bool STI::Network::convert<std::shared_ptr<FileServer>, TFileServer_var>(
        const std::shared_ptr<FileServer>& fileServer, TFileServer_var& tFileServer)
{
    return NetworkFileServer::getTFileServerReference(fileServer, tFileServer);
}
