#include "TMonitorManager_i.h"
#include <sti/device/MonitorManager.h>
#include "convert/Convert_Monitor.h"
#include <sti/device/Monitor.h>
#include <sti/utils/MixedValue.h>

#include <vector>
#include <string>
#include <memory>

using STI::TNetwork::TMonitorManager_i;
using STI::Network::convert;
using STI::TNetwork::TMonitorSeq;
using STI::Device::Monitor;
using STI::TNetwork::TMonitor;
using STI::Device::MonitorStatus;
using STI::TNetwork::TMonitorStatus;
using STI::TNetwork::TMixedValue;
using STI::Utils::MixedValue;


TMonitorManager_i::TMonitorManager_i(const std::shared_ptr<STI::Device::Device>& device)
{
    if (device != 0) {
        device->getMonitorManager(monitorManager);        
    }
}

TMonitorManager_i::~TMonitorManager_i()
{
}

::CORBA::Boolean TMonitorManager_i::getIDs(::STI::TNetwork::TStringSeq_out ids)
{
    STI::TNetwork::TStringSeq_var tStringSeq_var(new STI::TNetwork::TStringSeq);
    std::vector<std::string> localIDs;
    bool success = false;

    ids = new STI::TNetwork::TStringSeq();

    if (monitorManager != 0) {
        success = monitorManager->getIDs(localIDs);
        convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(localIDs, tStringSeq_var);

        (*ids) = tStringSeq_var;
    }

    return success;
}

::CORBA::Boolean TMonitorManager_i::getMonitor(const char* id, ::STI::TNetwork::TMonitor_out monitor)
{
    bool success = false;

    std::shared_ptr<Monitor> localMonitor;
    monitor = new TMonitor();

    if (monitorManager != 0) {
        success = monitorManager->getMonitor(id, localMonitor) && localMonitor != 0;
    }

    if (success) {
        success = convert<std::shared_ptr<Monitor>, TMonitor>(localMonitor, (TMonitor&)(*monitor));
    }

    return success;
}

::CORBA::Boolean TMonitorManager_i::getMonitors(::STI::TNetwork::TMonitorSeq_out monitors)
{
	std::vector<std::shared_ptr<Monitor>> localMonitors;
    bool success = false;
	monitors = new STI::TNetwork::TMonitorSeq();

	if (monitorManager != 0) {
		success = monitorManager->getMonitors(localMonitors);

		STI::TNetwork::TMonitorSeq_var tMonitorSeq_var(new STI::TNetwork::TMonitorSeq);

		convert<std::shared_ptr<Monitor>, TMonitor>(localMonitors,
			(_CORBA_Unbounded_Sequence<TMonitor>&) tMonitorSeq_var);

		(*monitors) = tMonitorSeq_var;
	}

    return success;
}

TMonitorStatus TMonitorManager_i::getStatus(const char* id)
{
    TMonitorStatus tStatus = TMonitorStatus::MonitorMissing;

    if (monitorManager != 0) {
        auto status = monitorManager->getStatus(id);
        tStatus = convert<MonitorStatus, TMonitorStatus>(status);
    }
    return tStatus;
}

void TMonitorManager_i::getValue(const char* id, ::STI::TNetwork::TMixedValue_out value)
{
    MixedValue dataOut;
    value = new STI::TNetwork::TMixedValue();

    if (monitorManager != 0) {
		dataOut = monitorManager->getValue(id);
	}

    (*value) = convert<MixedValue, TMixedValue>(dataOut);
}

void TMonitorManager_i::activate(const char* id)
{
    if (monitorManager != 0) {
        monitorManager->activate(id);
	}
}

void TMonitorManager_i::deactivate(const char* id)
{
    if (monitorManager != 0) {
        monitorManager->deactivate(id);
	}
}

void TMonitorManager_i::activateAll()
{
    if (monitorManager != 0) {
        monitorManager->activateAll();
    }
}

void TMonitorManager_i::deactivateAll()
{
    if (monitorManager != 0) {
        monitorManager->deactivateAll();
    }
}

::CORBA::Boolean TMonitorManager_i::ping()
{
    return true;
}
