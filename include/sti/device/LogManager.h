#ifndef STI_DEVICE_LOGMANAGER_H
#define STI_DEVICE_LOGMANAGER_H

#include <sti/device/LogID.h>
#include <sti/device/LogFile.h>
#include <sti/device/LogFileFilter.h>
#include <sti/device/LogRecord.h>

#include <sti/device/DeviceID.h>
#include <sti/utils/TimeStamp.h>

#include <string>
#include <vector>
#include <set>


namespace STI
{
namespace Device
{


class LogManager
{
public:

    virtual ~LogManager() {}

    virtual void getLogNames(std::set<std::string>& names) = 0;

    virtual int getLogCount(const DeviceID& deviceID, const LogFileFilter& filter) = 0;
    virtual void getLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids) = 0;
    virtual void getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogID>& ids) = 0;
    
    virtual bool getLog(const LogID& id, LogFile& logFile) = 0;
    virtual bool getLog(const std::string& name, const std::string& date, unsigned index, LogFile& logFile) = 0;

    virtual bool getLogs(const LogFileFilter& filter, std::vector<LogFile>& files) = 0;
    virtual bool getLogs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogFile>& files) = 0;

    virtual bool getLogRecord(const std::string& date, LogRecord& record) = 0;

};


} //Device
} //STI

#endif
