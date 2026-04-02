
#include "TLogManager_i.h"

#include "NetworkConvert.h"
#include "convert/Convert_Log.h"

using STI::TNetwork::TLogManager_i;
using STI::Network::convert;
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


TLogManager_i::TLogManager_i(const std::shared_ptr<STI::Device::Device>& device)
{
    if (device != 0) {
        device->getLogManager(logManager);
    }
}

TLogManager_i::~TLogManager_i()
{
}

void TLogManager_i::getLogNames(::STI::TNetwork::TStringSeq_out names)
{
    STI::TNetwork::TStringSeq_var tStringSeq_var(new STI::TNetwork::TStringSeq);
    std::set<std::string> localNames;

    names = new STI::TNetwork::TStringSeq();

    if (logManager != 0) {

        logManager->getLogNames(localNames);

        std::vector<std::string> namesVec;
        for (auto name : localNames) {
            namesVec.push_back(name);      //deep copy, but names localNames is short
        }
        convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(namesVec, tStringSeq_var);

        (*names) = tStringSeq_var;
    }
}

void TLogManager_i::getNetworkLogNames(::STI::TNetwork::TStringSeq_out names)
{
    STI::TNetwork::TStringSeq_var tStringSeq_var(new STI::TNetwork::TStringSeq);
    std::set<std::string> localNames;

    names = new STI::TNetwork::TStringSeq();

    if (logManager != 0) {

        logManager->getNetworkLogNames(localNames);

        std::vector<std::string> namesVec;
        for (const auto& name : localNames) {
            namesVec.push_back(name);
        }
        convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(namesVec, tStringSeq_var);

        (*names) = tStringSeq_var;
    }
}

::CORBA::Long TLogManager_i::getLogCount(const ::STI::TNetwork::TDeviceID& deviceID, const ::STI::TNetwork::TLogFileFilter& filter)
{
	::CORBA::Long count = 0;

    if (logManager != 0) {

		auto result = logManager->getLogCount(
                convert<TDeviceID, DeviceID>(deviceID), 
                convert<TLogFileFilter, LogFileFilter>(filter));

        count = static_cast<::CORBA::Long>(result);
	}
	return count;
}

::CORBA::Long TLogManager_i::getNetworkLogCount(const ::STI::TNetwork::TLogFileFilter& filter)
{
    ::CORBA::Long count = 0;

    if (logManager != 0) {
        auto result = logManager->getNetworkLogCount(convert<TLogFileFilter, LogFileFilter>(filter));
        count = static_cast<::CORBA::Long>(result);
    }

    return count;
}

void TLogManager_i::getLogIDs(const ::STI::TNetwork::TLogFileFilter& filter, ::STI::TNetwork::TLogIDSeq_out ids)
{
	std::vector<LogID> logIDs;
	ids = new STI::TNetwork::TLogIDSeq();

	if (logManager != 0) {
		logManager->getLogIDs(convert<TLogFileFilter, LogFileFilter>(filter), logIDs);

		STI::TNetwork::TLogIDSeq_var tLogIDSeq_var(new STI::TNetwork::TLogIDSeq);

		convert<LogID, STI::TNetwork::TLogID>(logIDs,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TLogID>&) tLogIDSeq_var);

		(*ids) = tLogIDSeq_var;
	}
}

void TLogManager_i::getNetworkLogIDs(const ::STI::TNetwork::TLogFileFilter& filter, ::STI::TNetwork::TLogIDSeq_out ids)
{
    std::vector<LogID> logIDs;
    ids = new STI::TNetwork::TLogIDSeq();

    if (logManager != 0) {
        logManager->getNetworkLogIDs(convert<TLogFileFilter, LogFileFilter>(filter), logIDs);

        STI::TNetwork::TLogIDSeq_var tLogIDSeq_var(new STI::TNetwork::TLogIDSeq);

        convert<LogID, STI::TNetwork::TLogID>(logIDs,
            (_CORBA_Unbounded_Sequence<STI::TNetwork::TLogID>&) tLogIDSeq_var);

        (*ids) = tLogIDSeq_var;
    }
}

void TLogManager_i::getDeviceLogIDs(const ::STI::TNetwork::TDeviceID& deviceID, const ::STI::TNetwork::TLogFileFilter& filter, ::STI::TNetwork::TLogIDSeq_out ids)
{
	std::vector<LogID> logIDs;
	ids = new STI::TNetwork::TLogIDSeq();

	if (logManager != 0) {
		logManager->getLogIDs(convert<TDeviceID, DeviceID>(deviceID), convert<TLogFileFilter, LogFileFilter>(filter), logIDs);

		STI::TNetwork::TLogIDSeq_var tLogIDSeq_var(new STI::TNetwork::TLogIDSeq);

		convert<LogID, STI::TNetwork::TLogID>(logIDs,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TLogID>&) tLogIDSeq_var);

		(*ids) = tLogIDSeq_var;
	}
}


::CORBA::Boolean TLogManager_i::getLog(const ::STI::TNetwork::TLogID& logID, ::STI::TNetwork::TLogFile_out logFile)
{
    bool success = false;

    LogFile localLogFile;
    logFile = new TLogFile();

    if (logManager != 0) {

        success = logManager->getLog(convert<TLogID, LogID>(logID), localLogFile);
    }

    if (success) {

        success = convert<LogFile, TLogFile>(localLogFile, (TLogFile&)(*logFile));
    }

    return success;
}



::CORBA::Boolean TLogManager_i::getLogs(const ::STI::TNetwork::TLogFileFilter& filter, ::STI::TNetwork::TLogFileSeq_out files)
{
    bool success = false;

	std::vector<LogFile> localLogFiles;
	files = new STI::TNetwork::TLogFileSeq();

	if (logManager != 0) {
		logManager->getLogs(convert<TLogFileFilter, LogFileFilter>(filter), localLogFiles);

		STI::TNetwork::TLogFileSeq_var tLogFileSeq_var(new STI::TNetwork::TLogFileSeq);

		convert<LogFile, TLogFile>(localLogFiles,
			(_CORBA_Unbounded_Sequence<TLogFile>&) tLogFileSeq_var);

		(*files) = tLogFileSeq_var;

        success = true;
    }
    return success;
}

::CORBA::Boolean TLogManager_i::getNetworkLogs(const ::STI::TNetwork::TLogFileFilter& filter, ::STI::TNetwork::TLogFileSeq_out files)
{
    bool success = false;

    std::vector<LogFile> localLogFiles;
    files = new STI::TNetwork::TLogFileSeq();

    if (logManager != 0) {
        success = logManager->getNetworkLogs(convert<TLogFileFilter, LogFileFilter>(filter), localLogFiles);

        STI::TNetwork::TLogFileSeq_var tLogFileSeq_var(new STI::TNetwork::TLogFileSeq);

        convert<LogFile, TLogFile>(localLogFiles,
            (_CORBA_Unbounded_Sequence<TLogFile>&) tLogFileSeq_var);

        (*files) = tLogFileSeq_var;
    }

    return success;
}

::CORBA::Boolean TLogManager_i::getDeviceLogs(const TDeviceID& deviceID, const TLogFileFilter& filter, ::STI::TNetwork::TLogFileSeq_out files)
{
    bool success = false;

	std::vector<LogFile> localLogFiles;
	files = new STI::TNetwork::TLogFileSeq();

	if (logManager != 0) {
		logManager->getLogs(convert<TDeviceID, DeviceID>(deviceID), convert<TLogFileFilter, LogFileFilter>(filter), localLogFiles);

		STI::TNetwork::TLogFileSeq_var tLogFileSeq_var(new STI::TNetwork::TLogFileSeq);

		convert<LogFile, TLogFile>(localLogFiles,
			(_CORBA_Unbounded_Sequence<TLogFile>&) tLogFileSeq_var);

		(*files) = tLogFileSeq_var;

        success = true;
	}
    return success;
}


::CORBA::Boolean TLogManager_i::getLogRecord(const char* date, ::STI::TNetwork::TLogRecord_out record)
{
    bool success = false;

    LogRecord localLogRecord;
    record = new TLogRecord();

    if (logManager != 0) {

        success = logManager->getLogRecord(date, localLogRecord);
    }

    if (success) {

        success = convert<LogRecord, TLogRecord>(localLogRecord, (TLogRecord&)(*record));
    }

    return success;
}

::CORBA::Boolean TLogManager_i::ping()
{
	return true;
}
