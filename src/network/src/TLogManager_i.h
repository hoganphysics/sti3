#ifndef STI_TNETWORK_TLOGMANAGER_I_H
#define STI_TNETWORK_TLOGMANAGER_I_H

#include <sti/device/LogManager.h>
#include <sti/device/Device.h>
#include "generated/deviceNet.h"

#include <memory>


namespace STI
{
namespace TNetwork
{


class TLogManager_i : public POA_STI::TNetwork::TLogManager
{
public:

    TLogManager_i(const std::shared_ptr<STI::Device::Device>& device);
	~TLogManager_i();
    
    void getLogNames(::STI::TNetwork::TStringSeq_out names);
    ::CORBA::Long getLogCount(const ::STI::TNetwork::TDeviceID& deviceID, const ::STI::TNetwork::TLogFileFilter& filter);
    void getLogIDs(const ::STI::TNetwork::TLogFileFilter& filter, ::STI::TNetwork::TLogIDSeq_out ids);
    void getDeviceLogIDs(const ::STI::TNetwork::TDeviceID& deviceID, const ::STI::TNetwork::TLogFileFilter& filter, ::STI::TNetwork::TLogIDSeq_out ids);
    ::CORBA::Boolean getLog(const ::STI::TNetwork::TLogID& logID, ::STI::TNetwork::TLogFile_out logFile);
    ::CORBA::Boolean getLogs(const ::STI::TNetwork::TLogFileFilter& filter, ::STI::TNetwork::TLogFileSeq_out files);
    ::CORBA::Boolean getDeviceLogs(const ::STI::TNetwork::TDeviceID& deviceID, const ::STI::TNetwork::TLogFileFilter& filter, ::STI::TNetwork::TLogFileSeq_out files);
    ::CORBA::Boolean getLogRecord(const char* date, ::STI::TNetwork::TLogRecord_out record);
    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Device::LogManager> logManager;

};


} //TNetwork
} //STI

#endif

