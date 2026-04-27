
#include "RemoteFileServer.h"
#include "TFileHolderRefInterface.h"

#include "convert/Convert_File.h"

using STI::Network::RemoteFileServer;
using STI::Utils::FileTransferType;
using STI::TNetwork::TFileTransferType;
using STI::Utils::FileID;
using STI::TNetwork::TFileID;
using STI::Network::TFileServerRefInterface;
using STI::TNetwork::TFileServer;


RemoteFileServer::RemoteFileServer(::STI::TNetwork::TFileServer_var fileServer)
: STI::TNetwork::TReferenceHolder<TFileServer>(fileServer)
{
}

RemoteFileServer::~RemoteFileServer()
{
}

bool RemoteFileServer::getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer)
{
	std::unique_lock<std::mutex> serverLock(fileServerMutex);

	if (isDisabled()) return false;

	tFileServer = STI::TNetwork::TFileServer::_duplicate(getTRef());

	return !CORBA::is_nil(tFileServer);
}


bool RemoteFileServer::findFile(const FileID& fileID)
{
    std::unique_lock<std::mutex> serverLock(fileServerMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->findFile(convert<FileID, TFileID>(fileID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return success;
}

int RemoteFileServer::getFileSize(const FileID& fileID)
{
    std::unique_lock<std::mutex> serverLock(fileServerMutex);

	if (isDisabled()) return 0;

    ::CORBA::Long fileSize = 0;

	try {
		getTRef()->getFileSize(convert<FileID, TFileID>(fileID), fileSize);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return fileSize;
}

bool RemoteFileServer::transferFile(const FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, FileTransferType type)
{
    std::unique_lock<std::mutex> serverLock(fileServerMutex);

	if (isDisabled()) return 0;

	bool result = false;

    STI::TNetwork::TFileHolder_var tDestination;

    if (!TFileHolderRefInterface::getTFileHolderReference(destination, tDestination)) {
        return false;
    }

	try {
		result = getTRef()->transferFile(
                                convert<FileID, TFileID>(source),
                                tDestination,
                                convert<FileTransferType, TFileTransferType>(type));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return result;
}

bool RemoteFileServer::transferFilePartial(const FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, int offset, int lines)
{
    std::unique_lock<std::mutex> serverLock(fileServerMutex);

	if (isDisabled()) return false;

	bool result = false;

    STI::TNetwork::TFileHolder_var tDestination;

    if (!TFileHolderRefInterface::getTFileHolderReference(destination, tDestination)) {
        return false;
    }

	try {
		result = getTRef()->transferFilePartial(
                                convert<FileID, TFileID>(source),
                                tDestination,
                                static_cast<CORBA::Long>(offset),
                                static_cast<CORBA::Long>(lines));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return result;
}

bool RemoteFileServer::deleteFile(const FileID& fileID)
{
    std::unique_lock<std::mutex> serverLock(fileServerMutex);

	if (isDisabled()) return false;

	bool result = false;

	try {
		result = getTRef()->deleteFile(convert<FileID, TFileID>(fileID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return result;
}
