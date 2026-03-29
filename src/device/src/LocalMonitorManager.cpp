#include "LocalMonitorManager.h"

#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/LocalMonitor.h>
#include <sti/device/MonitorListener.h>

#include "DeviceMessageGrouper.h"

#include <memory>
#include <map>
#include <mutex>
#include <vector>

using STI::Device::DeviceID;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::LocalMonitor;
using STI::Device::LocalMonitorManager;
using STI::Device::Monitor;
using STI::Device::MonitorListener;
using STI::Device::MonitorStatus;
using STI::Device::MonitorUpdateMessage;
using STI::Utils::MixedValue;

namespace STI
{
namespace Device
{

class LocalMonitorManagerState
{
public:
    using MonitorListenerList = std::vector<std::shared_ptr<MonitorListener>>;
    using ValueListener = std::function<void(const std::string&, const MixedValue&)>;
    using ValueListenerList = std::vector<ValueListener>;

    LocalMonitorManagerState(const DeviceID& localID, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
        : localID(localID),
          messageGrouper(dispatcher),
          listenerSnapshot(std::make_shared<MonitorListenerList>()),
          valueListenerSnapshot(std::make_shared<ValueListenerList>())
    {
        messageGrouper.setWarmup(100);
        messageGrouper.setCooldown(500);
        messageGrouper.start();
    }

    std::size_t registerMonitor(const std::string& id)
    {
        std::lock_guard<std::mutex> lock(listenerMutex);
        const auto generation = ++nextGeneration;
        monitorGenerations[id] = generation;
        return generation;
    }

    void unregisterMonitor(const std::string& id)
    {
        std::lock_guard<std::mutex> lock(listenerMutex);
        monitorGenerations.erase(id);
    }

    void handleActivated(const std::string& id, std::size_t generation)
    {
        std::shared_ptr<const MonitorListenerList> listeners;

        {
            std::lock_guard<std::mutex> lock(listenerMutex);

            if (!matchesRegistration(id, generation)) {
                return;
            }

            listeners = listenerSnapshot;
        }

        for (auto& listener : *listeners) {
            if (listener != 0) {
                listener->activated(id);
            }
        }
    }

    void handleDeactivated(const std::string& id, std::size_t generation)
    {
        std::shared_ptr<const MonitorListenerList> listeners;

        {
            std::lock_guard<std::mutex> lock(listenerMutex);

            if (!matchesRegistration(id, generation)) {
                return;
            }

            listeners = listenerSnapshot;
        }

        for (auto& listener : *listeners) {
            if (listener != 0) {
                listener->deactivated(id);
            }
        }
    }

    void handleValueUpdated(const std::string& id, const MixedValue& value, std::size_t generation)
    {
        std::shared_ptr<const MonitorListenerList> listeners;
        std::shared_ptr<const ValueListenerList> valueListeners;

        {
            std::lock_guard<std::mutex> lock(listenerMutex);

            if (!matchesRegistration(id, generation)) {
                return;
            }

            listeners = listenerSnapshot;
            valueListeners = valueListenerSnapshot;
        }

        auto message = std::make_shared<MonitorUpdateMessage>(localID, id, value);
        messageGrouper.addMessage(message);

        for (auto& listener : *listeners) {
            if (listener != 0) {
                listener->valueUpdated(id, value);
            }
        }

        for (auto& listener : *valueListeners) {
            if (listener) {
                listener(id, value);
            }
        }
    }

    void addListener(const std::shared_ptr<MonitorListener>& listener)
    {
        if (listener == 0) {
            return;
        }

        std::lock_guard<std::mutex> lock(listenerMutex);

        auto updatedSnapshot = std::make_shared<MonitorListenerList>();

        if (listenerSnapshot != 0) {
            *updatedSnapshot = *listenerSnapshot;
        }

        updatedSnapshot->push_back(listener);
        listenerSnapshot = updatedSnapshot;
    }

    void addValueListener(const ValueListener& listener)
    {
        if (!listener) {
            return;
        }

        std::lock_guard<std::mutex> lock(listenerMutex);

        auto updatedSnapshot = std::make_shared<ValueListenerList>();

        if (valueListenerSnapshot != 0) {
            *updatedSnapshot = *valueListenerSnapshot;
        }

        updatedSnapshot->push_back(listener);
        valueListenerSnapshot = updatedSnapshot;
    }

private:
    bool matchesRegistration(const std::string& id, std::size_t generation) const
    {
        const auto it = monitorGenerations.find(id);
        return it != monitorGenerations.end() && it->second == generation;
    }

    DeviceID localID;
    DeviceMessageGrouper<MonitorUpdateMessage> messageGrouper;
    std::map<std::string, std::size_t> monitorGenerations;
    std::shared_ptr<const MonitorListenerList> listenerSnapshot;
    std::shared_ptr<const ValueListenerList> valueListenerSnapshot;
    std::size_t nextGeneration{0};
    mutable std::mutex listenerMutex;
};


class LocalMonitorManagerForwarder : public MonitorListener
{
public:
    LocalMonitorManagerForwarder(const std::shared_ptr<LocalMonitorManagerState>& state, std::size_t generation)
        : state(state), generation(generation)
    {
    }

    void activated(const std::string& id) override
    {
        if (auto locked = state.lock()) {
            locked->handleActivated(id, generation);
        }
    }

    void deactivated(const std::string& id) override
    {
        if (auto locked = state.lock()) {
            locked->handleDeactivated(id, generation);
        }
    }

    void valueUpdated(const std::string& id, const MixedValue& value) override
    {
        if (auto locked = state.lock()) {
            locked->handleValueUpdated(id, value, generation);
        }
    }

private:
    std::weak_ptr<LocalMonitorManagerState> state;
    std::size_t generation;
};

} // namespace Device
} // namespace STI


LocalMonitorManager::LocalMonitorManager(const DeviceID& localID,
                                         const std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
    : state(std::make_shared<STI::Device::LocalMonitorManagerState>(localID, dispatcher))
{
}


void LocalMonitorManager::addMonitor(const std::shared_ptr<Monitor>& monitor)
{
    if (monitor == 0) {
        return;
    }

    const auto id = monitor->getID();

    if (id.empty() || monitors.contains(id)) {
        return;
    }

    if (monitors.add(id, monitor) && state != 0) {
        const auto generation = state->registerMonitor(id);
        monitor->addListener(std::make_shared<STI::Device::LocalMonitorManagerForwarder>(state, generation));
    }
}

void LocalMonitorManager::removeMonitor(const std::string& id)
{
    if (state != 0) {
        state->unregisterMonitor(id);
    }

    monitors.remove(id);
}

bool LocalMonitorManager::getIDs(std::vector<std::string>& ids)
{
    std::set<std::string> keys;
    monitors.getKeys(keys);

    ids.assign(keys.begin(), keys.end());
    return !ids.empty();
}

bool LocalMonitorManager::getMonitor(const std::string& id, std::shared_ptr<Monitor>& monitor)
{
    return (monitors.get(id, monitor) && monitor != 0);
}

bool LocalMonitorManager::getMonitors(std::vector<std::shared_ptr<Monitor>>& monitorList)
{
    monitorList.clear();

    std::vector<std::shared_ptr<Monitor>> values;
    monitors.getValues(values);

    for (auto& monitor : values) {
        if (monitor != 0) {
            monitorList.push_back(monitor);
        }
    }

    return !monitorList.empty();
}

MonitorStatus LocalMonitorManager::getStatus(const std::string& id) const
{
    std::shared_ptr<Monitor> monitor;

    if (monitors.get(id, monitor) && monitor != 0) {
        return monitor->getStatus();
    }

    return MonitorStatus::Missing;
}

MixedValue LocalMonitorManager::getValue(const std::string& key)
{
    std::shared_ptr<Monitor> monitor;

    if (getMonitor(key, monitor) && monitor != 0) {
        return monitor->getValue();
    }

    return MixedValue();
}

bool LocalMonitorManager::setValue(const std::string& key, const MixedValue& value)
{
    std::shared_ptr<Monitor> monitor;

    if (!getMonitor(key, monitor) || monitor == 0) {
        return false;
    }

    auto localMonitor = std::dynamic_pointer_cast<LocalMonitor>(monitor);

    if (localMonitor == 0) {
        return false;
    }

    localMonitor->setValue(value);
    return true;
}

void LocalMonitorManager::activate(const std::string& id)
{
    std::shared_ptr<Monitor> monitor;

    if (getMonitor(id, monitor) && monitor != 0) {
        monitor->activate();
    }
}

void LocalMonitorManager::deactivate(const std::string& id)
{
    std::shared_ptr<Monitor> monitor;

    if (getMonitor(id, monitor) && monitor != 0) {
        monitor->deactivate();
    }
}

void LocalMonitorManager::activateAll()
{
    std::vector<std::shared_ptr<Monitor>> monitorList;

    if (!getMonitors(monitorList)) {
        return;
    }

    for (auto& monitor : monitorList) {
        if (monitor != 0) {
            monitor->activate();
        }
    }
}

void LocalMonitorManager::deactivateAll()
{
    std::vector<std::shared_ptr<Monitor>> monitorList;

    if (!getMonitors(monitorList)) {
        return;
    }

    for (auto& monitor : monitorList) {
        if (monitor != 0) {
            monitor->deactivate();
        }
    }
}

void LocalMonitorManager::addListener(const std::shared_ptr<MonitorListener>& listener)
{
    if (state != 0) {
        state->addListener(listener);
    }
}

void LocalMonitorManager::addValueListener(
    const std::function<void(const std::string&, const STI::Utils::MixedValue&)>& listener)
{
    if (state != 0) {
        state->addValueListener(listener);
    }
}
