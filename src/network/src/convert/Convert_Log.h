
#ifndef STI_NETWORK_CONVERT_LOG_H
#define STI_NETWORK_CONVERT_LOG_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include "generated/orbTypes.h"

#include <sti/device/LogFile.h>
#include <sti/device/LogFileFilter.h>
#include <sti/device/LogRecord.h>

#include <memory>
#include <vector>


namespace STI
{

namespace Device
{
    class LogID;
    class LogFileFilter;
    struct LogFileRecord;
    struct LogNameRecord;
    class DeviceLogRecord;
    class LogFile;

} //Device



//LogID
template<>
Device::LogID Network::convert<TNetwork::TLogID, Device::LogID>(const TNetwork::TLogID& tLogID);
template<>
TNetwork::TLogID Network::convert<Device::LogID, TNetwork::TLogID>(const Device::LogID& logID);

template<>
bool Network::convert<TNetwork::TLogID, Device::LogID>(const TNetwork::TLogID& tLogID, Device::LogID& logID);
template<>
bool Network::convert<Device::LogID, TNetwork::TLogID>(const Device::LogID& logID, TNetwork::TLogID& tLogID);


//LogRecordStatus
template<>
Device::LogRecordStatus Network::convert<TNetwork::TLogRecordStatus, Device::LogRecordStatus>(const TNetwork::TLogRecordStatus& tStatus);
template<>
TNetwork::TLogRecordStatus Network::convert<Device::LogRecordStatus, TNetwork::TLogRecordStatus>(const Device::LogRecordStatus& status);


//LogFileRecord
template<>
bool Network::convert<TNetwork::TLogFileRecord, Device::LogFileRecord>(const TNetwork::TLogFileRecord& tLogFileRecord, Device::LogFileRecord& logFileRecord);
template<>
bool Network::convert<Device::LogFileRecord, TNetwork::TLogFileRecord>(const Device::LogFileRecord& logFileRecord, TNetwork::TLogFileRecord& tLogFileRecord);


//LogNameRecord
template<>
bool Network::convert<TNetwork::TLogNameRecord, Device::LogNameRecord>(const TNetwork::TLogNameRecord& tLogNameRecord, Device::LogNameRecord& logNameRecord);
template<>
bool Network::convert<Device::LogNameRecord, TNetwork::TLogNameRecord>(const Device::LogNameRecord& logNameRecord, TNetwork::TLogNameRecord& tLogNameRecord);


//LogFileFilter
template<>
Device::LogFileFilter Network::convert<TNetwork::TLogFileFilter, Device::LogFileFilter>(const TNetwork::TLogFileFilter& tFilter);
template<>
TNetwork::TLogFileFilter Network::convert<Device::LogFileFilter, TNetwork::TLogFileFilter>(const Device::LogFileFilter& filter);

template<>
bool Network::convert<TNetwork::TLogFileFilter, Device::LogFileFilter>(const TNetwork::TLogFileFilter& tFilter, Device::LogFileFilter& filter);
template<>
bool Network::convert<Device::LogFileFilter, TNetwork::TLogFileFilter>(const Device::LogFileFilter& filter, TNetwork::TLogFileFilter& tFilter);


//DeviceLogRecord
template<>
bool Network::convert<TNetwork::TDeviceLogRecord, Device::DeviceLogRecord>(const TNetwork::TDeviceLogRecord& tDeviceLogRecord, Device::DeviceLogRecord& deviceLogRecord);
template<>
bool Network::convert<Device::DeviceLogRecord, TNetwork::TDeviceLogRecord>(const Device::DeviceLogRecord& deviceLogRecord, TNetwork::TDeviceLogRecord& tDeviceLogRecord);


//LogRecord
template<>
bool Network::convert<TNetwork::TLogRecord, Device::LogRecord>(const TNetwork::TLogRecord& tLogRecord, Device::LogRecord& logRecord);
template<>
bool Network::convert<Device::LogRecord, TNetwork::TLogRecord>(const Device::LogRecord& logRecord, TNetwork::TLogRecord& tLogRecord);


//LogFileType
template<>
Device::LogFile::LogFileType Network::convert<TNetwork::TLogFileType, Device::LogFile::LogFileType>(const TNetwork::TLogFileType& tLogFileType);
template<>
TNetwork::TLogFileType Network::convert<Device::LogFile::LogFileType, TNetwork::TLogFileType>(const Device::LogFile::LogFileType& logFileType);


//LogFile
template<>
Device::LogFile Network::convert<TNetwork::TLogFile, Device::LogFile>(const TNetwork::TLogFile& tLogFile);
template<>
TNetwork::TLogFile Network::convert<Device::LogFile, TNetwork::TLogFile>(const Device::LogFile& logFile);

template<>
bool Network::convert<TNetwork::TLogFile, Device::LogFile>(const TNetwork::TLogFile& tLogFile, Device::LogFile& logFile);
template<>
bool Network::convert<Device::LogFile, TNetwork::TLogFile>(const Device::LogFile& logFile, TNetwork::TLogFile& tLogFile);




} //STI

#endif
