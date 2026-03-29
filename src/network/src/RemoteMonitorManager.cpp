
#include "RemoteMonitorManager.h"

#include "DeviceMessageListenerForwarder.h"
#include "RemoteMonitor.h"

#include "convert/Convert_Monitor.h"

#include <sti/device/MonitorListener.h>

#include <string>
#include <utility>
#include <vector>

using STI::Network::convert;
using STI::Network::RemoteMonitorManager;
using STI::Network::RemoteMonitor;
using STI::Device::Monitor;
using STI::Device::MonitorListener;
using STI::Device::MonitorStatus;
using STI::Device::MonitorUpdateMessage;
using STI::Device::MonitorStatusUpdateMessage;
using STI::Utils::MixedValue;
using STI::TNetwork::TMonitorManager;
using STI::TNetwork::TMonitor;
using STI::TNetwork::TMonitorSeq;
using STI::TNetwork::TMonitorStatus;
using STI::TNetwork::TMixedValue;


RemoteMonitorManager::RemoteMonitorManager(::STI::TNetwork::TMonitorManager_var manager,
                                           const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder,
                                           const STI::Device::DeviceID& remoteID)
    : TReferenceHolder<::STI::TNetwork::TMonitorManager>(manager),
      remoteID(remoteID),
      listenerForwarder(forwarder),
      listenerSnapshot(std::make_shared<ListenerList>())
{
    std::shared_ptr<STI::Device::DeviceMessageListener<MonitorUpdateMessage>> valueListener;
    valueListener = std::make_shared<MonitorUpdater>(this);

    std::shared_ptr<STI::Device::DeviceMessageListener<MonitorStatusUpdateMessage>> statusListener;
    statusListener = std::make_shared<MonitorStatusUpdater>(this);

    valueListenerID.name = "RemoteMonitorManager::MonitorUpdate";
    valueListenerID.type = STI::Device::DeviceMessageType::MonitorUpdate;

    statusListenerID.name = "RemoteMonitorManager::MonitorStatusUpdate";
    statusListenerID.type = STI::Device::DeviceMessageType::MonitorStatusUpdate;

    if (listenerForwarder != 0) {
        listenerForwarder->addListener(remoteID, valueListenerID, valueListener);
        listenerForwarder->addListener(remoteID, statusListenerID, statusListener);
    }

    std::vector<std::shared_ptr<Monitor>> monitors;
    getMonitors(monitors);
}

RemoteMonitorManager::~RemoteMonitorManager()
{
    if (listenerForwarder != 0) {
        listenerForwarder->removeListener(remoteID, valueListenerID);
        listenerForwarder->removeListener(remoteID, statusListenerID);
    }
}

bool RemoteMonitorManager::getIDs(std::vector<std::string>& ids)
{
	std::unique_lock<std::mutex> monitorLock(monitorMutex);

	ids.clear();

	if (isDisabled()) return false;

	STI::TNetwork::TStringSeq_var tIDs(new STI::TNetwork::TStringSeq);
    bool success = false;

	try {
		success = getTRef()->getIDs(tIDs);	//remote call

        if (success) {
		    convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tIDs, ids);
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success && !ids.empty();
}


bool RemoteMonitorManager::getMonitor(const std::string& id, std::shared_ptr<STI::Device::Monitor>& monitor)
{
	std::unique_lock<std::mutex> monitorLock(monitorMutex);

    monitor.reset();

	if (isDisabled()) return false;

	STI::TNetwork::TMonitor_var tMonitor(new STI::TNetwork::TMonitor);
    std::shared_ptr<RemoteMonitor> remoteMonitor;
    bool success = false;

	try {
		success = getTRef()->getMonitor(convert<std::string, ::CORBA::String_member>(id), tMonitor);	//remote call

        if (success) {
            success = convert<TMonitor, std::shared_ptr<RemoteMonitor>>(tMonitor, remoteMonitor);
        }

        if (success && remoteMonitor != 0) {
            remoteMonitor->attachManager(this);
            setMonitorData(remoteMonitor);
            monitor = std::static_pointer_cast<Monitor>(remoteMonitor);
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success && (monitor != 0);
}

bool RemoteMonitorManager::getMonitors(std::vector<std::shared_ptr<STI::Device::Monitor>>& monitors)
{
	std::unique_lock<std::mutex> monitorLock(monitorMutex);

    monitors.clear();

	if (isDisabled()) return false;

	STI::TNetwork::TMonitorSeq_var tMonitors(new TMonitorSeq);
    std::vector<std::shared_ptr<RemoteMonitor>> remoteMonitors;
    bool success = false;

	try {
		success = getTRef()->getMonitors(tMonitors);	//remote call

        if (success) {
		    success = convert<TMonitor, std::shared_ptr<RemoteMonitor>>(tMonitors, remoteMonitors);
        }

        if (success) {
            monitorData.clear();

            for (auto& remoteMonitor : remoteMonitors) {
                if (remoteMonitor != 0) {
                    remoteMonitor->attachManager(this);
                    setMonitorData(remoteMonitor);
                    monitors.push_back(std::static_pointer_cast<Monitor>(remoteMonitor));
                }
            }
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success && !monitors.empty();
}

MonitorStatus RemoteMonitorManager::getStatus(const std::string& id) const
{
	std::unique_lock<std::mutex> monitorLock(monitorMutex);

    MonitorStatus status = MonitorStatus::Missing;

	if (isDisabled()) return status;
		
	try {
		auto tStatus = getTRef()->getStatus(
				convert<std::string, ::CORBA::String_member>(id));	//remote call
		status = convert<TMonitorStatus, MonitorStatus>(tStatus);
        monitorData[id].status = status;
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
    return status;
}

STI::Utils::MixedValue RemoteMonitorManager::getValue(const std::string& id)
{
	std::unique_lock<std::mutex> monitorLock(monitorMutex);

	if (isDisabled()) return STI::Utils::MixedValue();

	STI::TNetwork::TMixedValue_var tValue;
    MixedValue value;

	try {
		getTRef()->getValue(convert<std::string, ::CORBA::String_member>(id), tValue);	//remote call
        value = convert<TMixedValue, MixedValue>(tValue);
        monitorData[id].value = value;
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return value;
}

void RemoteMonitorManager::activate(const std::string& id)
{
    std::shared_ptr<const ListenerList> listeners;
    bool notify = false;

    {
	    std::unique_lock<std::mutex> monitorLock(monitorMutex);

	    if (isDisabled()) return;

        bool success = false;

	    try {
		    getTRef()->activate(convert<std::string, ::CORBA::String_member>(id));	//remote call
            success = true;
	    }
	    catch (CORBA::TRANSIENT&) {
	    }
	    catch (CORBA::SystemException&) {
	    }
	    catch (CORBA::Exception&) {
	    }

        if (!success) {
            return;
        }

        auto& data = monitorData[id];
        notify = data.status != MonitorStatus::Active;
        data.status = MonitorStatus::Active;
        listeners = listenerSnapshot;
    }

    if (notify && listeners != 0) {
        for (auto& listener : *listeners) {
            if (listener != 0) {
                listener->activated(id);
            }
        }
    }
}

void RemoteMonitorManager::deactivate(const std::string& id)
{
    std::shared_ptr<const ListenerList> listeners;
    bool notify = false;

    {
	    std::unique_lock<std::mutex> monitorLock(monitorMutex);

	    if (isDisabled()) return;

        bool success = false;

	    try {
		    getTRef()->deactivate(convert<std::string, ::CORBA::String_member>(id));	//remote call
            success = true;
	    }
	    catch (CORBA::TRANSIENT&) {
	    }
	    catch (CORBA::SystemException&) {
	    }
	    catch (CORBA::Exception&) {
	    }

        if (!success) {
            return;
        }

        auto& data = monitorData[id];
        notify = data.status != MonitorStatus::Inactive;
        data.status = MonitorStatus::Inactive;
        listeners = listenerSnapshot;
    }

    if (notify && listeners != 0) {
        for (auto& listener : *listeners) {
            if (listener != 0) {
                listener->deactivated(id);
            }
        }
    }
}

void RemoteMonitorManager::activateAll()
{
    std::shared_ptr<const ListenerList> listeners;
    std::vector<std::string> activatedIDs;

    {
	    std::unique_lock<std::mutex> monitorLock(monitorMutex);

	    if (isDisabled()) return;

        bool success = false;

	    try {
		    getTRef()->activateAll();	//remote call
            success = true;
	    }
	    catch (CORBA::TRANSIENT&) {
	    }
	    catch (CORBA::SystemException&) {
	    }
	    catch (CORBA::Exception&) {
	    }

        if (!success) {
            return;
        }

        for (auto& entry : monitorData) {
            if (entry.second.status != MonitorStatus::Active) {
                entry.second.status = MonitorStatus::Active;
                activatedIDs.push_back(entry.first);
            }
        }

        listeners = listenerSnapshot;
    }

    if (listeners != 0) {
        for (auto& id : activatedIDs) {
            for (auto& listener : *listeners) {
                if (listener != 0) {
                    listener->activated(id);
                }
            }
        }
    }
}

void RemoteMonitorManager::deactivateAll()
{
    std::shared_ptr<const ListenerList> listeners;
    std::vector<std::string> deactivatedIDs;

    {
	    std::unique_lock<std::mutex> monitorLock(monitorMutex);

	    if (isDisabled()) return;

        bool success = false;

	    try {
		    getTRef()->deactivateAll();	//remote call
            success = true;
	    }
	    catch (CORBA::TRANSIENT&) {
	    }
	    catch (CORBA::SystemException&) {
	    }
	    catch (CORBA::Exception&) {
	    }

        if (!success) {
            return;
        }

        for (auto& entry : monitorData) {
            if (entry.second.status != MonitorStatus::Inactive) {
                entry.second.status = MonitorStatus::Inactive;
                deactivatedIDs.push_back(entry.first);
            }
        }

        listeners = listenerSnapshot;
    }

    if (listeners != 0) {
        for (auto& id : deactivatedIDs) {
            for (auto& listener : *listeners) {
                if (listener != 0) {
                    listener->deactivated(id);
                }
            }
        }
    }
}

void RemoteMonitorManager::addListener(const std::shared_ptr<STI::Device::MonitorListener>& listener)
{
    if (listener == 0) {
        return;
    }

    std::unique_lock<std::mutex> monitorLock(monitorMutex);

    auto updatedSnapshot = std::make_shared<ListenerList>();

    if (listenerSnapshot != 0) {
        *updatedSnapshot = *listenerSnapshot;
    }

    updatedSnapshot->push_back(listener);
    listenerSnapshot = updatedSnapshot;
}


bool RemoteMonitorManager::ping() const
{
	std::unique_lock<std::mutex> monitorLock(monitorMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}

void RemoteMonitorManager::setMonitorData(const std::shared_ptr<RemoteMonitor>& monitor)
{
    if (monitor == 0) {
        return;
    }

    auto& data = monitorData[monitor->getID()];
    data.status = monitor->getStoredStatus();
    data.value = monitor->getStoredValue();
}

MixedValue RemoteMonitorManager::getUpdatedValue(const std::string& id) const
{
    std::unique_lock<std::mutex> monitorLock(monitorMutex);

    auto it = monitorData.find(id);

    if (it != monitorData.end()) {
        return it->second.value;
    }

    return MixedValue();
}

void RemoteMonitorManager::handleMessage(const std::shared_ptr<MonitorUpdateMessage>& mess)
{
    if (mess == 0) {
        return;
    }

    std::shared_ptr<const ListenerList> listeners;
    std::vector<std::pair<std::string, MixedValue>> updates;

    {
        std::unique_lock<std::mutex> monitorLock(monitorMutex);

        for (auto& update : mess->updates) {
            auto& data = monitorData[update.first];
            data.value = update.second;
            updates.emplace_back(update.first, update.second);
        }

        listeners = listenerSnapshot;
    }

    if (listeners == 0) {
        return;
    }

    for (auto& update : updates) {
        for (auto& listener : *listeners) {
            if (listener != 0) {
                listener->valueUpdated(update.first, update.second);
            }
        }
    }
}

void RemoteMonitorManager::handleMessage(const std::shared_ptr<MonitorStatusUpdateMessage>& mess)
{
    if (mess == 0) {
        return;
    }

    std::shared_ptr<const ListenerList> listeners;
    std::vector<std::pair<std::string, MonitorStatus>> updates;

    {
        std::unique_lock<std::mutex> monitorLock(monitorMutex);

        for (auto& update : mess->updates) {
            auto& data = monitorData[update.first];

            if (data.status != update.second) {
                updates.emplace_back(update.first, update.second);
            }

            data.status = update.second;
        }

        listeners = listenerSnapshot;
    }

    if (listeners == 0) {
        return;
    }

    for (auto& update : updates) {
        for (auto& listener : *listeners) {
            if (listener == 0) {
                continue;
            }

            switch (update.second) {
            case MonitorStatus::Active:
                listener->activated(update.first);
                break;
            case MonitorStatus::Inactive:
                listener->deactivated(update.first);
                break;
            case MonitorStatus::Missing:
                break;
            }
        }
    }
}
