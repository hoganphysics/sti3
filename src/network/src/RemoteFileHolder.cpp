#include "RemoteFileHolder.h"
#include "NetworkConvert.h"
#include "TFileHolderRefInterface.h"
#include "convert/Convert_File.h"


using STI::Network::RemoteFileHolder;
using ::STI::TNetwork::TFileHolder_var;
using ::STI::TNetwork::TFileHolder;
using STI::Utils::FileHolder;
using STI::Network::convertBuffer;


RemoteFileHolder::RemoteFileHolder(TFileHolder_var fileHolder)
: STI::TNetwork::TReferenceHolder<TFileHolder>(fileHolder)
{
}

RemoteFileHolder::~RemoteFileHolder()
{
}

bool RemoteFileHolder::getTFileHolderRef(STI::TNetwork::TFileHolder_var& tFileHolder)
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (isDisabled()) return false;

	tFileHolder = STI::TNetwork::TFileHolder::_duplicate(getTRef());

	return !CORBA::is_nil(tFileHolder);
}

STI::Utils::FileID RemoteFileHolder::getID() const
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	::STI::TNetwork::TFileID_var tFileID;
	bool success = false;

	if (!isDisabled()) {
		try {
			tFileID = getTRef()->getID();
			success = true;
		}
		catch (CORBA::TRANSIENT&) {
		}
		catch (CORBA::SystemException&) {
		}
		catch (CORBA::Exception&)
		{
		}
	}

	STI::Utils::FileID fileID;

	if(success) {
		fileID = convert<::STI::TNetwork::TFileID, STI::Utils::FileID>(tFileID);
	}

	return fileID;
}

std::string RemoteFileHolder::getFilename() const
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (filename.isCached()) return filename.get();

	if (isDisabled()) return "";

	std::string result = "";

	try {
		result = getTRef()->getFilename();	//remote call
		filename.set(result);
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

unsigned RemoteFileHolder::getFileSize() const
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (fileSize.isCached()) return fileSize.get();

	if (isDisabled()) return 0;

	unsigned result = 0;

	try {
		result = static_cast<unsigned>(getTRef()->getFileSize());	//remote call
		fileSize.set(result);
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

bool RemoteFileHolder::exists() const
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (fileExists.isCached()) return fileExists.get();

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->exists();	//remote call
		fileExists.set(success);
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


bool RemoteFileHolder::transferFile(const std::shared_ptr<FileHolder>& destination)
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (isDisabled()) return false;

	bool success = false;

    STI::TNetwork::TFileHolder_var tDestination;

    if (!TFileHolderRefInterface::getTFileHolderReference(destination, tDestination)) {
        return false;
    }

	try {
		success = getTRef()->transferFile(tDestination);	//remote call
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

unsigned RemoteFileHolder::maxBufferSize() const
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (bufferSize.isCached()) return bufferSize.get();

	if (isDisabled()) return false;

	unsigned result = 32*1000;  //default

	try {
		result = getTRef()->maxBufferSize();	//remote call
		bufferSize.set(result);
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

std::string RemoteFileHolder::md5Checksum()
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (checksum.isCached()) return checksum.get();

	if (isDisabled()) return "";

	std::string result = "";

	try {
		result = getTRef()->md5Checksum();	//remote call
		checksum.set(result);
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

bool RemoteFileHolder::openFile()
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->openFile();	//remote call
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

bool RemoteFileHolder::write(const char* buffer, unsigned length)
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
        ::STI::TNetwork::OctetSeq tBuffer;
        convertBuffer(buffer, length, tBuffer);

		success = getTRef()->write(tBuffer);	//remote call
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

void RemoteFileHolder::closeFile()
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (isDisabled()) return;

	try {
		getTRef()->closeFile();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}
