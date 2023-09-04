#ifndef STI_NETWORK_REMOTELOGMANAGER_H
#define STI_NETWORK_REMOTELOGMANAGER_H

#include "deviceNet.h"
#include <sti/device/LogManager.h>
#include "TReferenceHolder.h"

#include <mutex>
#include <string>


namespace STI
{
namespace Network
{

class RemoteLogManager : public STI::Device::LogManager,
					     public STI::TNetwork::TReferenceHolder<STI::TNetwork::TLogManager>	//mixin
{
public:

    RemoteLogManager(::STI::TNetwork::TLogManager_ptr manager, const STI::Device::DeviceID& deviceID);
    ~RemoteLogManager();

    void getLogNames(std::set<std::string>& names);

    int getLogCount(const STI::Device::LogFileFilter& filter);
    int getLogCount(const STI::Device::DeviceID& deviceID, const STI::Device::LogFileFilter& filter);
    void getLogIDs(const STI::Device::LogFileFilter& filter, std::vector<STI::Device::LogID>& ids);
    void getLogIDs(const STI::Device::DeviceID& deviceID, const STI::Device::LogFileFilter& filter, std::vector<STI::Device::LogID>& ids);
    
    bool getLog(const STI::Device::LogID& id, STI::Device::LogFile& logFile);
    bool getLog(const std::string& name, const std::string& date, unsigned index, STI::Device::LogFile& logFile);

    bool getLogs(const STI::Device::LogFileFilter& filter, std::vector<STI::Device::LogFile>& files);
    bool getLogs(const STI::Device::DeviceID& deviceID, const STI::Device::LogFileFilter& filter, std::vector<STI::Device::LogFile>& files);

    bool getLogRecord(const std::string& date, STI::Device::LogRecord& record);

    bool ping() const;
    
private:

    STI::Device::DeviceID remoteDeviceID;
	mutable std::mutex logMutex;
};


} //Network
} //STI


#endif
