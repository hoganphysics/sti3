#ifndef STI_TNETWORK_TMONITORMANAGER_I_H
#define STI_TNETWORK_TMONITORMANAGER_I_H

#include <sti/device/MonitorManager.h>

#include <sti/device/Device.h>
#include "generated/deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{


class TMonitorManager_i : public POA_STI::TNetwork::TMonitorManager, 
                            public PortableServer::RefCountServantBase
{
public:

	TMonitorManager_i(const std::shared_ptr<STI::Device::Device>& device);
	~TMonitorManager_i();

    ::CORBA::Boolean getIDs(::STI::TNetwork::TStringSeq_out ids);
    ::CORBA::Boolean getMonitor(const char* id, ::STI::TNetwork::TMonitor_out monitor);
    ::CORBA::Boolean getMonitors(::STI::TNetwork::TMonitorSeq_out monitors);
    TMonitorStatus getStatus(const char* id);
    void getValue(const char* id, ::STI::TNetwork::TMixedValue_out value);
    void activate(const char* id);
    void deactivate(const char* id);
    void activateAll();
    void deactivateAll();
    ::CORBA::Boolean ping();
    
private:

    std::shared_ptr<STI::Device::MonitorManager> monitorManager;

};


} //TNetwork
} //STI

#endif

