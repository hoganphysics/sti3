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
using STI::Device::ImportStorage;
using STI::Device::ImportCollisionPolicy;
using STI::Device::ImportLifetime;
using STI::Device::ImportFileOptions;
using STI::TNetwork::TImportStorage;
using STI::TNetwork::TImportCollisionPolicy;
using STI::TNetwork::TImportLifetime;
using STI::TNetwork::TImportFileOptions;


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

//Import options
template<>
ImportStorage STI::Network::convert<TImportStorage, ImportStorage>(const TImportStorage& tStorage)
{
    switch (tStorage) {
    case TImportStorage::ImportStorageVirtual:
        return ImportStorage::Virtual;
    case TImportStorage::ImportStorageDiskTemporary:
    default:
        return ImportStorage::DiskTemporary;
    }
}

template<>
TImportStorage STI::Network::convert<ImportStorage, TImportStorage>(const ImportStorage& storage)
{
    switch (storage) {
    case ImportStorage::Virtual:
        return TImportStorage::ImportStorageVirtual;
    case ImportStorage::DiskTemporary:
    default:
        return TImportStorage::ImportStorageDiskTemporary;
    }
}

template<>
ImportCollisionPolicy STI::Network::convert<TImportCollisionPolicy, ImportCollisionPolicy>(const TImportCollisionPolicy& tCollision)
{
    switch (tCollision) {
    case TImportCollisionPolicy::ImportCollisionFailIfExists:
        return ImportCollisionPolicy::FailIfExists;
    case TImportCollisionPolicy::ImportCollisionReplace:
        return ImportCollisionPolicy::Replace;
    case TImportCollisionPolicy::ImportCollisionUnique:
    default:
        return ImportCollisionPolicy::Unique;
    }
}

template<>
TImportCollisionPolicy STI::Network::convert<ImportCollisionPolicy, TImportCollisionPolicy>(const ImportCollisionPolicy& collision)
{
    switch (collision) {
    case ImportCollisionPolicy::FailIfExists:
        return TImportCollisionPolicy::ImportCollisionFailIfExists;
    case ImportCollisionPolicy::Replace:
        return TImportCollisionPolicy::ImportCollisionReplace;
    case ImportCollisionPolicy::Unique:
    default:
        return TImportCollisionPolicy::ImportCollisionUnique;
    }
}

template<>
ImportLifetime STI::Network::convert<TImportLifetime, ImportLifetime>(const TImportLifetime&)
{
    return ImportLifetime::Handle;
}

template<>
TImportLifetime STI::Network::convert<ImportLifetime, TImportLifetime>(const ImportLifetime&)
{
    return TImportLifetime::ImportLifetimeHandle;
}

template<>
ImportFileOptions STI::Network::convert<TImportFileOptions, ImportFileOptions>(const TImportFileOptions& tOptions)
{
    ImportFileOptions options;
    convert<TImportFileOptions, ImportFileOptions>(tOptions, options);
    return options;
}

template<>
TImportFileOptions STI::Network::convert<ImportFileOptions, TImportFileOptions>(const ImportFileOptions& options)
{
    TImportFileOptions tOptions;
    convert<ImportFileOptions, TImportFileOptions>(options, tOptions);
    return tOptions;
}

template<>
bool STI::Network::convert<TImportFileOptions, ImportFileOptions>(const TImportFileOptions& tOptions, ImportFileOptions& options)
{
    options.storage = convert<TImportStorage, ImportStorage>(tOptions.storage);
    options.collision = convert<TImportCollisionPolicy, ImportCollisionPolicy>(tOptions.collision);
    options.lifetime = convert<TImportLifetime, ImportLifetime>(tOptions.lifetime);
    options.ttl = std::chrono::seconds(tOptions.ttlSeconds);
    return true;
}

template<>
bool STI::Network::convert<ImportFileOptions, TImportFileOptions>(const ImportFileOptions& options, TImportFileOptions& tOptions)
{
    tOptions.storage = convert<ImportStorage, TImportStorage>(options.storage);
    tOptions.collision = convert<ImportCollisionPolicy, TImportCollisionPolicy>(options.collision);
    tOptions.lifetime = convert<ImportLifetime, TImportLifetime>(options.lifetime);
    tOptions.ttlSeconds = static_cast<CORBA::ULong>(options.ttl.count());
    return true;
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
