
#include "LogRecordFile.h"

#include <sti/device/LogRecord.h>

#include "CerealArchives.h"

#include <algorithm>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

using STI::Device::LogRecord;
using STI::Device::LogRecordFile;
using STI::Utils::TimeStamp;


LogRecordFile::LogRecordFile(const std::string& filename)
: filename(filename)
{
    load();
}


LogRecordFile::~LogRecordFile()
{
}


void LogRecordFile::setLogStatus(const DeviceID& id, const std::string& logName)
{
    auto& records = logRecord.deviceLogRecords;

    auto record = records.find(id.getID());

    if (record == records.end()) {
        //new
        auto& newRecord = records[id.getID()];
        newRecord.deviceID = id.getID();
        newRecord.logNames.insert(logName);
        newRecord.logs[logName].logName = logName;
        newRecord.logs[logName].lastUpdate = TimeStamp();
        newRecord.status = LogRecordStatus::LogsPresent;
        newRecord.syncLogNamesFromLogs();
    }
    else {
        record->second.status = LogRecordStatus::LogsPresent;
        record->second.logNames.insert(logName);
        record->second.logs[logName].logName = logName;
        record->second.logs[logName].lastUpdate = TimeStamp();
        record->second.syncLogNamesFromLogs();
    }

    logRecord.timeStamp = TimeStamp();
    save();
}

void LogRecordFile::save()
{
    std::ofstream file( filename );
    cereal::XMLOutputArchive archive( file );

    archive(logRecord);
}

void LogRecordFile::load()
{
    if (!exists()) return;

    std::ifstream file( filename );
    
    if (!file.is_open()) return;
    if (!file.good() || file.peek() == std::ifstream::traits_type::eof()) {
        // file exists but is empty
        return;
    }

    try {
        cereal::XMLInputArchive archive( file );

        archive(logRecord);
    }
    catch (const cereal::Exception &e) {
        //failed to load - likely due to version mismatch or corruption
        return;
    }
}

bool LogRecordFile::exists() const
{
    fs::path logRecordPath = filename;
    return std::filesystem::exists(logRecordPath);
}

LogRecord& LogRecordFile::getRecord()
{
    return logRecord;
}

const LogRecord& LogRecordFile::getRecord() const
{
    return logRecord;
}

bool LogRecordFile::copyRecord(LogRecord& record)
{
    if (exists()) {
        record = logRecord;
        return true;
    }
    return false;
}
