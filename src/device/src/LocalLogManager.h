#ifndef STI_DEVICE_LOCALLOGMANAGER_H
#define STI_DEVICE_LOCALLOGMANAGER_H

#include <sti/device/LogManager.h>
#include <sti/utils/SynchronizedMap.h>
#include <sti/utils/TimeStamp.h>

#include <sti/device/GroupableMessage.h>
#include <sti/device/MessageGrouper.h>
#include <sti/utils/EventQueue.h>

#include <string>
#include <vector>
#include <memory>

namespace STI
{
namespace Device
{

class Logger;
class LocalTaskManager;
class LocalAttributeManager;
class LocalChannelManager;
class LocalDevice;
class LocalPersistenceManager;
class LogRecordFile;


class LocalLogManager : public LogManager
{
public:

    LocalLogManager(LocalDevice* localDevice, const std::shared_ptr<LocalPersistenceManager>& localPersistenceManager);
    ~LocalLogManager();

    void getLogNames(std::set<std::string>& names);

    int getLogCount(const DeviceID& deviceID, const LogFileFilter& filter);
    void getLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids);
    void getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogID>& ids);
    
    bool getLog(const LogID& id, LogFile& logFile);
    bool getLog(const std::string& name, const std::string& date, unsigned index, LogFile& logFile);

    bool getLogs(const LogFileFilter& filter, std::vector<LogFile>& files);
    bool getLogs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogFile>& files);

    bool getLogRecord(const std::string& date, LogRecord& record);

    void createLogger(const std::string& name);
    
    Logger& log();
    Logger& log(const std::string& name);

private:

    bool getLogRecordFile(const STI::Utils::TimeStamp& timestamp, std::shared_ptr<LogRecordFile>& recordFile, bool autocreate=false);

    // int getLogCount(const STI::Utils::TimeStamp& date, const DeviceID& deviceID, const std::string& logName);
    bool getLogCounts(const STI::Utils::TimeStamp& date, const DeviceID& deviceID, std::map<std::string, int>& counts);
    void getLogIDs(const STI::Utils::TimeStamp& date, const DeviceID& deviceID, const std::string& logName, int startIndex, int endIndex, std::vector<LogID>& ids);

    void writeLog(const std::string& logName);      //save to disk

    std::string makeLogFilename(const std::string& logName, int index);
    bool getLogName(const std::string& filenameStem, std::string& logName, int& index) const;

    class LogWriteMessage : public STI::Device::GroupableMessage<LogWriteMessage>
    {
    public:

        LogWriteMessage(const std::string& logName)
        {
            logNames.insert(logName);
        }
        ~LogWriteMessage() {}

        bool appendMessage(const LogWriteMessage& mess) 
        {
            logNames.insert(mess.logNames.begin(), mess.logNames.end());
            return true;
        }
        bool groupable() const { return true; }
        LogWriteMessage& get() { return *this; }

        std::set<std::string> logNames;
    };

    class LogWriterEventQueue : public STI::Utils::EventQueue<std::shared_ptr<LogWriteMessage>>
    {
    public:
        
        LogWriterEventQueue(LocalLogManager* manager) : self(manager) { start(); }

        void handleEvent(const std::shared_ptr<LogWriteMessage>& mess)
        {
            if (mess == 0) return;

            for (auto& name : mess->logNames) {
                self->writeLog(name);
            }
        }
        LocalLogManager* self;
    };

    class LogWriterMessageGrouper : public MessageGrouper<LogWriteMessage>
    {
    public:

        LogWriterMessageGrouper(LocalLogManager* manager) : logWriterEventQueue(manager) {}

        void dispatchMessage(const std::shared_ptr<LogWriteMessage>& mess)
        {
            logWriterEventQueue.addEvent(mess);
            // if (mess == 0) return;

            // for (auto& name : mess->logNames) {
            //     self->writeLog(name);
            // }
        }
        LogWriterEventQueue logWriterEventQueue;
        // LocalLogManager* self;
    };




    friend class Logger;

    LocalDevice* localDevice;
    std::shared_ptr<LocalPersistenceManager> localPersistenceManager;
    LogWriterMessageGrouper logWriterMessageGrouper;

    STI::Utils::SynchronizedMap<std::string, std::shared_ptr<Logger>> loggers;

    std::string lastLogBasePath;
    std::shared_ptr<LogRecordFile> lastLogRecordFile;

};


} //Device
} //STI

#endif
