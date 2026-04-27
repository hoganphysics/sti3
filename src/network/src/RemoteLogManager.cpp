
#include "RemoteLogManager.h"

#include "generated/orbTypes.h"
#include "convert/Convert_Log.h"

using STI::Network::convert;
using STI::Network::RemoteLogManager;
using STI::TNetwork::TReferenceHolder;
using STI::TNetwork::TLogManager;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Device::LogFileFilter;
using STI::TNetwork::TLogFileFilter;
using STI::Device::LogID;
using STI::TNetwork::TLogID;
using STI::Device::LogFile;
using STI::TNetwork::TLogFile;
using STI::TNetwork::TLogFile;
using STI::Device::LogRecord;
using STI::TNetwork::TLogRecord;



RemoteLogManager::RemoteLogManager(::STI::TNetwork::TLogManager_var manager, const STI::Device::DeviceID& deviceID)
: TReferenceHolder<TLogManager>(manager), remoteDeviceID(deviceID)
{
}

RemoteLogManager::~RemoteLogManager()
{
    disable();
}


void RemoteLogManager::getLogNames(std::set<std::string>& names)
{
	std::unique_lock<std::mutex> logLock(logMutex);

	if (isDisabled()) return;

	STI::TNetwork::TStringSeq_var tNames(new STI::TNetwork::TStringSeq);

	names.clear();

	try {
		getTRef()->getLogNames(tNames);	//remote call
		
		std::vector<std::string> namesVec;
		convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tNames, namesVec);		//only vector<string> is available
		
		names.insert(namesVec.begin(), namesVec.end());		//deep copy, but names should be very short
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}

void RemoteLogManager::getNetworkLogNames(std::set<std::string>& names)
{
    std::unique_lock<std::mutex> logLock(logMutex);

    if (isDisabled()) return;

    STI::TNetwork::TStringSeq_var tNames(new STI::TNetwork::TStringSeq);

    try {
        getTRef()->getNetworkLogNames(tNames);    //remote call

        std::vector<std::string> namesVec;
        convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tNames, namesVec);

        names.insert(namesVec.begin(), namesVec.end());
    }
    catch (CORBA::TRANSIENT&) {
    }
    catch (CORBA::SystemException&) {
    }
    catch (CORBA::Exception&) {
    }
}

int RemoteLogManager::getLogCount(const LogFileFilter& filter)
{
	return getLogCount(remoteDeviceID, filter);
}

int RemoteLogManager::getLogCount(const DeviceID& deviceID, const LogFileFilter& filter)
{
	std::unique_lock<std::mutex> logLock(logMutex);

	if (isDisabled()) return 0;

    int count = 0;

	try {
		auto result = getTRef()->getLogCount(
                convert<DeviceID, TDeviceID>(deviceID), 
                convert<LogFileFilter, TLogFileFilter>(filter));	//remote call
		
        count = static_cast<int>(result);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return count;
}

int RemoteLogManager::getNetworkLogCount(const LogFileFilter& filter)
{
    std::unique_lock<std::mutex> logLock(logMutex);

    if (isDisabled()) return 0;

    int count = 0;

    try {
        auto result = getTRef()->getNetworkLogCount(convert<LogFileFilter, TLogFileFilter>(filter));    //remote call
        count = static_cast<int>(result);
    }
    catch (CORBA::TRANSIENT&) {
    }
    catch (CORBA::SystemException&) {
    }
    catch (CORBA::Exception&) {
    }

    return count;
}

void RemoteLogManager::getLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids)
{
	std::unique_lock<std::mutex> logLock(logMutex);

	if (isDisabled()) return;
	
	STI::TNetwork::TLogIDSeq_var tIDs(new STI::TNetwork::TLogIDSeq);

	try {
		getTRef()->getLogIDs(convert<LogFileFilter, TLogFileFilter>(filter), tIDs);	//remote call

		convert<TLogID, LogID>(tIDs, ids);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteLogManager::getNetworkLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids)
{
    std::unique_lock<std::mutex> logLock(logMutex);

    if (isDisabled()) return;

    STI::TNetwork::TLogIDSeq_var tIDs(new STI::TNetwork::TLogIDSeq);

    try {
        getTRef()->getNetworkLogIDs(convert<LogFileFilter, TLogFileFilter>(filter), tIDs);    //remote call

        convert<TLogID, LogID>(tIDs, ids);
    }
    catch (CORBA::TRANSIENT&) {
    }
    catch (CORBA::SystemException&) {
    }
    catch (CORBA::Exception&) {
    }
}

void RemoteLogManager::getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogID>& ids)
{
    std::unique_lock<std::mutex> logLock(logMutex);

	if (isDisabled()) return;
	
	STI::TNetwork::TLogIDSeq_var tIDs(new STI::TNetwork::TLogIDSeq);

	try {
		getTRef()->getDeviceLogIDs(
                convert<DeviceID, TDeviceID>(deviceID), 
                convert<LogFileFilter, TLogFileFilter>(filter), 
                tIDs);	//remote call

		convert<TLogID, LogID>(tIDs, ids);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


bool RemoteLogManager::getLog(const LogID& id, LogFile& logFile)
{
	std::unique_lock<std::mutex> logLock(logMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TLogFile_var tLogFile(new STI::TNetwork::TLogFile);

	bool success = false;

	try {
		success = getTRef()->getLog(convert<LogID, TLogID>(id), tLogFile);	//remote call

		convert<TLogFile, LogFile>(tLogFile, logFile);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success;
}

bool RemoteLogManager::getLog(const std::string& name, const std::string& date, unsigned index, LogFile& logFile)
{
    LogID logID;
    logID.logName = name;
    logID.date = date;
    logID.index = index;
    logID.deviceID = remoteDeviceID;

    return getLog(logID, logFile);
}


bool RemoteLogManager::getLogs(const LogFileFilter& filter, std::vector<LogFile>& files)
{
    // return getLogs(remoteDeviceID, filter, files);

    std::unique_lock<std::mutex> logLock(logMutex);

	if (isDisabled()) return false;

    bool success = false;
	
	STI::TNetwork::TLogFileSeq_var tLogFiles(new STI::TNetwork::TLogFileSeq);

	try {
		success = getTRef()->getLogs(
                convert<LogFileFilter, TLogFileFilter>(filter), 
                tLogFiles);	//remote call

		success &= convert<TLogFile, LogFile>(tLogFiles, files);
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

bool RemoteLogManager::getNetworkLogs(const LogFileFilter& filter, std::vector<LogFile>& files)
{
    std::unique_lock<std::mutex> logLock(logMutex);

    if (isDisabled()) return false;

    bool success = false;

    STI::TNetwork::TLogFileSeq_var tLogFiles(new STI::TNetwork::TLogFileSeq);

    try {
        success = getTRef()->getNetworkLogs(convert<LogFileFilter, TLogFileFilter>(filter), tLogFiles);    //remote call

        success &= convert<TLogFile, LogFile>(tLogFiles, files);
    }
    catch (CORBA::TRANSIENT&) {
    }
    catch (CORBA::SystemException&) {
    }
    catch (CORBA::Exception&) {
    }

    return success;
}

bool RemoteLogManager::getLogs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogFile>& files)
{
    std::unique_lock<std::mutex> logLock(logMutex);

	if (isDisabled()) return false;

    bool success = false;
	
	STI::TNetwork::TLogFileSeq_var tLogFiles(new STI::TNetwork::TLogFileSeq);

	try {
		success = getTRef()->getDeviceLogs(
                convert<DeviceID, TDeviceID>(deviceID), 
                convert<LogFileFilter, TLogFileFilter>(filter), 
                tLogFiles);	//remote call

		success &= convert<TLogFile, LogFile>(tLogFiles, files);
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

bool RemoteLogManager::getLogRecord(const std::string& date, LogRecord& record)
{
	std::unique_lock<std::mutex> logLock(logMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TLogRecord_var tLogRecord(new STI::TNetwork::TLogRecord);

	bool success = false;

	try {
		success = getTRef()->getLogRecord(
            convert<std::string, ::CORBA::String_member>(date),
            tLogRecord);	//remote call

		convert<TLogRecord, LogRecord>(tLogRecord, record);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success;
}


bool RemoteLogManager::ping() const
{
	std::unique_lock<std::mutex> logLock(logMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
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
