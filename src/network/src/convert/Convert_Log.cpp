
#include "Convert_Log.h"
#include "Convert_EventEngine.h"
#include "Convert_ResultsCollector.h"
#include "Convert_File.h"


#include <sti/device/LogID.h>
#include <sti/device/LogRecord.h>
#include <sti/device/LogFile.h>
#include <sti/device/LogFileFilter.h>


using STI::Network::convert;

using STI::TNetwork::TLogID;
using STI::Device::LogID;
using STI::TNetwork::TLogRecordStatus;
using STI::Device::LogRecordStatus;
using STI::TNetwork::TLogFileFilter;
using STI::Device::LogFileFilter;
using STI::TNetwork::TDeviceLogRecord;
using STI::Device::DeviceLogRecord;
using STI::TNetwork::TLogRecord;
using STI::Device::LogRecord;
using STI::TNetwork::TLogFileType;

using STI::TNetwork::TLogFile;
using STI::Device::LogFile;

using STI::TNetwork::TFileID;
using STI::Utils::FileID;


//LogID
template<>
LogID STI::Network::convert<TLogID, LogID>(const TLogID& tLogID)
{
    LogID logID;
    convert<TLogID, LogID>(tLogID, logID);
    return logID;
}

template<>
TLogID STI::Network::convert<LogID, TLogID>(const LogID& logID)
{
    TLogID tLogID;
    convert<LogID, TLogID>(logID, tLogID);
    return tLogID;
}

template<>
bool STI::Network::convert<TLogID, LogID>(const TLogID& tLogID, LogID& logID)
{
    logID.deviceID = convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tLogID.deviceID);
    logID.date = convert<CORBA::String_member, std::string>(tLogID.date);
    logID.logName = convert<CORBA::String_member, std::string>(tLogID.logName);
    logID.index = static_cast<unsigned>(tLogID.index);
    return true;
}

template<>
bool STI::Network::convert<LogID, TLogID>(const LogID& logID, TLogID& tLogID)
{
    tLogID.deviceID = convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(logID.deviceID);
    tLogID.date = convert<std::string, CORBA::String_member>(logID.date);
    tLogID.logName = convert<std::string, CORBA::String_member>(logID.logName);
    tLogID.index = static_cast<CORBA::Long>(logID.index);
    return true;
}



//LogRecordStatus
template<>
LogRecordStatus STI::Network::convert<TLogRecordStatus, LogRecordStatus>(const TLogRecordStatus& tStatus)
{
    //LogRecordStatus { Unqueried, LogsPresent, NoLogs, Error };
    //                 LogRecordUnqueried, LogRecordLogsPresent, LogRecordNoLogs, LogRecordError

    LogRecordStatus status;

    switch (tStatus) 
    {
    case TLogRecordStatus::LogRecordUnqueried:
        status = LogRecordStatus::Unqueried;
        break;
    case TLogRecordStatus::LogRecordLogsPresent:
        status = LogRecordStatus::LogsPresent;
        break;
    case TLogRecordStatus::LogRecordNoLogs:
        status = LogRecordStatus::NoLogs;
        break;
    case TLogRecordStatus::LogRecordError:
        status = LogRecordStatus::Error;
        break;
    default:
        status = LogRecordStatus::Error;
        break;
    }
    return status;
}


template<>
TLogRecordStatus STI::Network::convert<LogRecordStatus, TLogRecordStatus>(const LogRecordStatus& status)
{
    TLogRecordStatus tStatus;

    switch (status) 
    {
    case LogRecordStatus::Unqueried:
        tStatus = TLogRecordStatus::LogRecordUnqueried;
        break;
    case LogRecordStatus::LogsPresent:
        tStatus = TLogRecordStatus::LogRecordLogsPresent;
        break;
    case LogRecordStatus::NoLogs:
        tStatus = TLogRecordStatus::LogRecordNoLogs;
        break;
    case LogRecordStatus::Error:
        tStatus = TLogRecordStatus::LogRecordError;
        break;
    default:
        tStatus = TLogRecordStatus::LogRecordError;
        break;
    }
    return tStatus;
}


//LogFileFilter

template<>
LogFileFilter STI::Network::convert<TLogFileFilter, LogFileFilter>(const TLogFileFilter& tFilter)
{
    LogFileFilter filter;
    convert<TLogFileFilter, LogFileFilter>(tFilter, filter);
    return filter;
}

template<>
TLogFileFilter STI::Network::convert<LogFileFilter, TLogFileFilter>(const LogFileFilter& filter)
{
    TLogFileFilter tFilter;
    convert<LogFileFilter, TLogFileFilter>(filter, tFilter);
    return tFilter;
}

template<>
bool STI::Network::convert<TLogFileFilter, LogFileFilter>(const TLogFileFilter& tFilter, LogFileFilter& filter)
{
    filter.logName = convert<CORBA::String_member, std::string>(tFilter.logName);
    filter.startDate = convert<CORBA::String_member, std::string>(tFilter.startDate);
    filter.endDate = convert<CORBA::String_member, std::string>(tFilter.endDate);
    filter.startIndex = static_cast<int>(tFilter.startIndex);
    filter.endIndex = static_cast<int>(tFilter.endIndex);
    return true;
}


template<>
bool STI::Network::convert<LogFileFilter, TLogFileFilter>(const LogFileFilter& filter, TLogFileFilter& tFilter)
{
    tFilter.logName = convert<std::string, CORBA::String_member>(filter.logName);
    tFilter.startDate = convert<std::string, CORBA::String_member>(filter.startDate);
    tFilter.endDate = convert<std::string, CORBA::String_member>(filter.endDate);
    tFilter.startIndex = static_cast<CORBA::Long>(filter.startIndex);
    tFilter.endIndex = static_cast<CORBA::Long>(filter.endIndex);
    return true;
}



//DeviceLogRecord
template<>
bool STI::Network::convert<TDeviceLogRecord, DeviceLogRecord>(const TDeviceLogRecord& tDeviceLogRecord, DeviceLogRecord& deviceLogRecord)
{
    deviceLogRecord.deviceID = convert<CORBA::String_member, std::string>(tDeviceLogRecord.deviceID);
    deviceLogRecord.status = convert<TLogRecordStatus, LogRecordStatus>(tDeviceLogRecord.status);

    std::vector<std::string> logNamesVec;
	convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tDeviceLogRecord.logNames, logNamesVec);		//only vector<string> is available
	deviceLogRecord.logNames.insert(logNamesVec.begin(), logNamesVec.end());		//deep copy
    return true;
}


template<>
bool STI::Network::convert<DeviceLogRecord, TDeviceLogRecord>(const DeviceLogRecord& deviceLogRecord, TDeviceLogRecord& tDeviceLogRecord)
{
    tDeviceLogRecord.deviceID = convert<std::string, CORBA::String_member>(deviceLogRecord.deviceID);
    tDeviceLogRecord.status = convert<LogRecordStatus, TLogRecordStatus>(deviceLogRecord.status);

    std::vector<std::string> logNamesVec;

    for (auto& name : deviceLogRecord.logNames) {
        logNamesVec.push_back(name);
    }
    
	convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(logNamesVec, tDeviceLogRecord.logNames);		//only vector<string> is available

    return true;
}




//LogRecord
template<>
bool STI::Network::convert<TLogRecord, LogRecord>(const TLogRecord& tLogRecord, LogRecord& logRecord)
{
    logRecord.timeStamp = convert<STI::TNetwork::TTimeStamp, STI::Utils::TimeStamp>(tLogRecord.timeStamp);

    for (unsigned i = 0; i < tLogRecord.deviceLogRecords.length(); ++i) {
        DeviceLogRecord deviceLogRecord;
        convert<TDeviceLogRecord, DeviceLogRecord>(tLogRecord.deviceLogRecords[i], deviceLogRecord);
        logRecord.deviceLogRecords[deviceLogRecord.deviceID] = deviceLogRecord;
    }

    return true;
}


template<>
bool STI::Network::convert<LogRecord, TLogRecord>(const LogRecord& logRecord, TLogRecord& tLogRecord)
{
    tLogRecord.timeStamp = convert<STI::Utils::TimeStamp, STI::TNetwork::TTimeStamp>(logRecord.timeStamp);

    tLogRecord.deviceLogRecords.length(logRecord.deviceLogRecords.size());

    unsigned i = 0;
    for (auto& deviceRecordLog : logRecord.deviceLogRecords) {
        convert<DeviceLogRecord, TDeviceLogRecord>(deviceRecordLog.second, tLogRecord.deviceLogRecords[i]);
        i++;
    }

    return true; 
}



//LogFileType
template<>
LogFile::LogFileType STI::Network::convert<TLogFileType, LogFile::LogFileType>(const TLogFileType& tLogFileType)
{
    //enum class LogFileType { FileID, FileHolder, String };

    LogFile::LogFileType type;
    
    switch (tLogFileType) 
    {
    case TLogFileType::LogFileFileID:
        type = LogFile::LogFileType::FileID;
        break;
    case TLogFileType::LogFileFileHolder:
        type = LogFile::LogFileType::FileHolder;
        break;
    case TLogFileType::LogFileString:
        type = LogFile::LogFileType::String;
        break;
    default:
        type = LogFile::LogFileType::FileHolder;
        break;
    }
    return type;
}


template<>
TLogFileType STI::Network::convert<LogFile::LogFileType, TLogFileType>(const LogFile::LogFileType& logFileType)
{
    TLogFileType tType;
    
    switch (logFileType) 
    {
    case LogFile::LogFileType::FileID:
        tType = TLogFileType::LogFileFileID;
        break;
    case LogFile::LogFileType::FileHolder:
        tType = TLogFileType::LogFileFileHolder;
        break;
    case LogFile::LogFileType::String:
        tType = TLogFileType::LogFileString;
        break;
    default:
        tType = TLogFileType::LogFileFileHolder;
        break;
    }
    return tType;
}


//LogFile
template<>
LogFile STI::Network::convert<TLogFile, LogFile>(const TLogFile& tLogFile)
{
    LogFile logFile;
    convert<TLogFile, LogFile>(tLogFile, logFile);
    return logFile;
}

template<>
TLogFile STI::Network::convert<LogFile, TLogFile>(const LogFile& logFile)
{
    TLogFile tLogFile;
    convert<LogFile, TLogFile>(logFile, tLogFile);
    return tLogFile;
}


template<>
bool STI::Network::convert<TLogFile, LogFile>(const TLogFile& tLogFile, LogFile& logFile)
{
    logFile.id = convert<TLogID, LogID>(tLogFile.logID);
    logFile.type = convert<TLogFileType, LogFile::LogFileType>(tLogFile.type);
    logFile.logString = convert<CORBA::String_member, std::string>(tLogFile.logString);
    logFile.fileID = convert<TFileID, FileID>(tLogFile.fileID);
    convert<STI::TNetwork::TFileHolder_var, std::shared_ptr<STI::Utils::FileHolder>>(tLogFile.fileHolder, logFile.fileHolder);
    return true;
}


template<>
bool STI::Network::convert<LogFile, TLogFile>(const LogFile& logFile, TLogFile& tLogFile)
{
    tLogFile.logID = convert<LogID, TLogID>(logFile.id);
    tLogFile.type = convert<LogFile::LogFileType, TLogFileType>(logFile.type);
    tLogFile.logString = convert<std::string, CORBA::String_member>(logFile.logString);
    tLogFile.fileID = convert<FileID, TFileID>(logFile.fileID);

    STI::TNetwork::TFileHolder_var tFileHolder;
    convert<std::shared_ptr<STI::Utils::FileHolder>, STI::TNetwork::TFileHolder_var>(logFile.fileHolder, tFileHolder);
    tLogFile.fileHolder = tFileHolder;

    return true;
}


