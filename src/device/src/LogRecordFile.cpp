
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


void LogRecordFile::setLogStatus(const DeviceID& id, LogRecordStatus status)
{
    auto& ids = logRecord.loggedIDs;
    auto it = ids.find(id.getID());

    if (it != ids.end() && it->second == status) {
        //no change
        return;
    }

    ids[id.getID()] = status;
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

    cereal::XMLInputArchive archive( file );

    archive(logRecord);
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

bool LogRecordFile::copyRecord(LogRecord& record)
{
    if (exists()) {
        record = logRecord;
        return true;
    }
    return false;
}



