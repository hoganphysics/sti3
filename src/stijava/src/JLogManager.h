
#ifndef STI_DEVICE_JLOGMANAGER_H
#define STI_DEVICE_JLOGMANAGER_H

#include <sti/device/LogManager.h>

#include <memory>
#include <string>


namespace STI
{
namespace Device
{

class LogManager;
class DeviceID;


//Java LogManager wrapper
class JLogManager
{
public:
	
	JLogManager(const std::shared_ptr<STI::Device::LogManager>& manager);
	~JLogManager();

    std::set<std::string> getLogNames();

    int getLogCount(const LogFileFilter& filter);
    int getLogCount(const DeviceID& deviceID, const LogFileFilter& filter);
    std::vector<LogID> getLogIDs(const LogFileFilter& filter);
    std::vector<LogID> getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter);
    
    LogFile getLog(const LogID& id);
    LogFile getLog(const std::string& name, const std::string& date, unsigned index);

    std::vector<LogFile> getLogs(const LogFileFilter& filter);
    std::vector<LogFile> getLogs(const DeviceID& deviceID, const LogFileFilter& filter);

    LogRecord getLogRecord(const std::string& date);

private:

    std::shared_ptr<STI::Device::LogManager> logManager;

};

} //Device
} //STI

#endif
