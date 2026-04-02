#include "TFileServer_i.h"

#include <sti/utils/FileID.h>
#include <sti/utils/FileServer.h>

#include "NetworkConvert.h"
#include "RemoteFileHolder.h"
#include "convert/Convert_File.h"


using STI::Network::convert;
using STI::TNetwork::TFileServer_i;
using ::STI::TNetwork::TFileID;
using STI::Utils::FileID;
using ::STI::TNetwork::TFileTransferType;
using STI::Utils::FileTransferType;


TFileServer_i::TFileServer_i(STI::Utils::FileServer* fileServer)
: localFileServer(fileServer)
{    
}

TFileServer_i::~TFileServer_i()
{
}


::CORBA::Boolean TFileServer_i::findFile(const ::STI::TNetwork::TFileID& fileID)
{
    bool result = false;

    if (localFileServer != 0) {
		result = localFileServer->findFile(convert<TFileID, FileID>(fileID));
	}
    return static_cast<::CORBA::Boolean>(result);
}

::CORBA::Boolean TFileServer_i::getFileSize(const ::STI::TNetwork::TFileID& fileID, ::CORBA::Long& fileSize)
{
    bool result = false;

    if (localFileServer != 0) {
        FileID localFileID = convert<TFileID, FileID>(fileID);
        
        if (localFileServer->findFile(localFileID)) {
            int size = localFileServer->getFileSize(localFileID);
            fileSize = static_cast<::CORBA::Long>(size);
            result = true;
        }
	}
    return static_cast<::CORBA::Boolean>(result);
}

::CORBA::Boolean TFileServer_i::transferFile(const ::STI::TNetwork::TFileID& source, ::STI::TNetwork::TFileHolder_ptr destination, ::STI::TNetwork::TFileTransferType type)
{
    bool result = false;

    if (localFileServer != 0 && !CORBA::is_nil(destination)) {
        
        STI::TNetwork::TFileHolder_var destination_var = STI::TNetwork::TFileHolder::_duplicate(destination);

        auto remoteFile = std::make_shared<STI::Network::RemoteFileHolder>(destination_var);
		result = localFileServer->transferFile(
                convert<TFileID, FileID>(source), 
                remoteFile, 
                convert<TFileTransferType, FileTransferType>(type));
	}
    return static_cast<::CORBA::Boolean>(result);
}

::CORBA::Boolean TFileServer_i::transferFilePartial(const ::STI::TNetwork::TFileID& source, ::STI::TNetwork::TFileHolder_ptr destination, ::CORBA::Long offset, ::CORBA::Long lines)
{
    bool result = false;

    if (localFileServer != 0 && !CORBA::is_nil(destination)) {
        STI::TNetwork::TFileHolder_var destination_var = STI::TNetwork::TFileHolder::_duplicate(destination);

        auto remoteFile = std::make_shared<STI::Network::RemoteFileHolder>(destination_var);
        result = localFileServer->transferFilePartial(
                convert<TFileID, FileID>(source),
                remoteFile,
                static_cast<int>(offset),
                static_cast<int>(lines));
    }

    return static_cast<::CORBA::Boolean>(result);
}

::CORBA::Boolean TFileServer_i::deleteFile(const ::STI::TNetwork::TFileID& fileID)
{
    bool result = false;

    if (localFileServer != 0) {
		result = localFileServer->deleteFile(convert<TFileID, FileID>(fileID));
	}
    return static_cast<::CORBA::Boolean>(result);
}
