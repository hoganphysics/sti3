#ifndef STI_DEVICE_LOCALLOGREPOSITORY_H
#define STI_DEVICE_LOCALLOGREPOSITORY_H

#include <sti/device/LogFile.h>
#include <sti/device/LogFileFilter.h>
#include <sti/device/LogID.h>
#include <sti/device/LogRecord.h>
#include <sti/utils/TimeStamp.h>

#include <memory>
#include <set>
#include <string>
#include <vector>


namespace STI
{
namespace Device
{

class LocalDevice;
class LocalPersistenceManager;
class LogCatalogFile;
class LogRecordFile;
struct LogNameRecord;

class LocalLogRepository
{
public:

    LocalLogRepository(LocalDevice* localDevice, const std::shared_ptr<LocalPersistenceManager>& localPersistenceManager);
    ~LocalLogRepository();

    void getLogNames(std::set<std::string>& names);

    int getLogCount(const LogFileFilter& filter);
    void getLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids);

    bool getLog(const LogID& id, LogFile& logFile);
    bool getLogs(const LogFileFilter& filter, std::vector<LogFile>& files);
    bool getLogRecord(const std::string& date, LogRecord& record);

    bool recordLocalWrite(const STI::Utils::TimeStamp& timestamp, const std::string& logFilename);

private:

    bool getLogRecordFile(const STI::Utils::TimeStamp& timestamp, std::shared_ptr<LogRecordFile>& recordFile, bool autocreate = false);
    bool getLogCatalogFile(std::shared_ptr<LogCatalogFile>& catalogFile);

    bool ensureRecordReady(const STI::Utils::TimeStamp& timestamp, LogRecordFile& recordFile);
    bool shouldRebuildRecord(const STI::Utils::TimeStamp& timestamp, const LogRecordFile& recordFile) const;
    bool rebuildDailyRecord(const STI::Utils::TimeStamp& timestamp, LogRecordFile& recordFile);
    bool rebuildCatalog(LogCatalogFile& catalogFile);
    bool catalogNeedsRebuild(const LogCatalogFile& catalogFile) const;

    bool buildDeviceLogRecord(const STI::Utils::TimeStamp& timestamp, DeviceLogRecord& deviceRecord) const;
    bool buildLogFileRecord(const STI::Utils::TimeStamp& timestamp, const std::string& logName, unsigned index,
        const std::string& logFilename, LogFileRecord& fileRecord) const;

    bool getDeviceLogPath(const STI::Utils::TimeStamp& timestamp, std::string& logDevicePath) const;
    bool getLogRootPath(std::string& logRootPath) const;

    std::size_t countMatchingFiles(const LogNameRecord& nameRecord, int startIndex, int endIndex) const;
    void appendMatchingIDs(const LogNameRecord& nameRecord, int startIndex, int endIndex, std::vector<LogID>& ids) const;

    LocalDevice* localDevice;
    std::shared_ptr<LocalPersistenceManager> localPersistenceManager;

    std::shared_ptr<LogRecordFile> lastLogRecordFile;
    STI::Utils::TimeStamp lastRecordDate;
    std::shared_ptr<LogCatalogFile> cachedCatalogFile;
};


} //Device
} //STI

#endif
