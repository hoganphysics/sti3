
#include "RemoteFileHolder.h"
#include "NetworkConvert.h"
#include "TFileHolderRefInterface.h"

using STI::Network::RemoteFileHolder;
using ::STI::TNetwork::TFileHolder_ptr;
using ::STI::TNetwork::TFileHolder;
using STI::Utils::FileHolder;
using STI::Network::convertBuffer;


RemoteFileHolder::RemoteFileHolder(TFileHolder_ptr fileHolder)
: STI::TNetwork::TReferenceHolder<TFileHolder>(fileHolder, fileMutex)
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

std::string RemoteFileHolder::getFilename() const
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (isDisabled()) return "";

	std::string result = "";

	try {
		result = getTRef()->getFilename();	//remote call
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

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->exists();	//remote call
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

	if (isDisabled()) return false;

	unsigned result = 32*1000;  //default

	try {
		result = getTRef()->maxBufferSize();	//remote call
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


bool RemoteFileHolder::deleteFile()
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->deleteFile();	//remote call
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


std::string RemoteFileHolder::md5Checksum()
{
	std::unique_lock<std::mutex> fileLock(fileMutex);

	if (isDisabled()) return "";

	std::string result = "";

	try {
		result = getTRef()->md5Checksum();	//remote call
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

