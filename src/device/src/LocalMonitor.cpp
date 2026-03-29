#include <sti/device/LocalMonitor.h>
#include <sti/device/MonitorListener.h>

#include <utility>

using STI::Device::LocalMonitor;
using STI::Device::MonitorListener;
using STI::Device::MonitorStatus;
using STI::Utils::MixedValue;

namespace
{

std::string parseMonitorGroup(const std::string& id)
{
    const auto pos = id.find_last_of('/');

    if (pos == std::string::npos) {
        return "";
    }

    return id.substr(0, pos);
}

} // namespace


LocalMonitor::LocalMonitor(const std::string& id)
    : id_(id),
      group_(parseMonitorGroup(id)),
      status_(MonitorStatus::Active),
      listenerSnapshot(std::make_shared<ListenerList>())
{
}


std::string LocalMonitor::getID() const
{
    std::unique_lock<std::mutex> lock(monitorMutex);
    return id_;
}

std::string LocalMonitor::getGroup() const
{
    std::unique_lock<std::mutex> lock(monitorMutex);
    return group_;
}

MonitorStatus LocalMonitor::getStatus() const
{
    std::unique_lock<std::mutex> lock(monitorMutex);
    return status_;
}

void LocalMonitor::activate()
{
    std::shared_ptr<const ListenerList> listeners;
    std::string id;

    {
        std::unique_lock<std::mutex> lock(monitorMutex);

        if (status_ == MonitorStatus::Active) {
            return;
        }

        status_ = MonitorStatus::Active;
        id = id_;
        listeners = listenerSnapshot;
    }

    for (auto& listener : *listeners) {
        if (listener != 0) {
            listener->activated(id);
        }
    }
}

void LocalMonitor::deactivate()
{
    std::shared_ptr<const ListenerList> listeners;
    std::string id;

    {
        std::unique_lock<std::mutex> lock(monitorMutex);

        if (status_ == MonitorStatus::Inactive) {
            return;
        }

        status_ = MonitorStatus::Inactive;
        id = id_;
        listeners = listenerSnapshot;
    }

    for (auto& listener : *listeners) {
        if (listener != 0) {
            listener->deactivated(id);
        }
    }
}

void LocalMonitor::setValue(const MixedValue& value)
{
    std::shared_ptr<const ListenerList> listeners;
    std::string id;
    bool notify = false;

    {
        std::unique_lock<std::mutex> lock(monitorMutex);

        value_ = value;
        notify = (status_ == MonitorStatus::Active);

        if (notify) {
            id = id_;
            listeners = listenerSnapshot;
        }
    }

    if (!notify) {
        return;
    }

    for (auto& listener : *listeners) {
        if (listener != 0) {
            listener->valueUpdated(id, value);
        }
    }
}

MixedValue LocalMonitor::getValue()
{
    std::unique_lock<std::mutex> lock(monitorMutex);
    return value_;
}

LocalMonitor& LocalMonitor::addMetaData(const std::string& key, const MixedValue& data)
{
    std::unique_lock<std::mutex> lock(monitorMutex);
    metaData.addMetaData(key, data);
    return *this;
}

const MixedValue& LocalMonitor::getMetaData() const
{
    std::unique_lock<std::mutex> lock(monitorMutex);
    return metaData.getMetaData();
}

MixedValue LocalMonitor::getMetaData(const std::string& key) const
{
    std::unique_lock<std::mutex> lock(monitorMutex);
    return metaData.getMetaData(key);
}

void LocalMonitor::addListener(const std::shared_ptr<MonitorListener>& listener)
{
    if (listener == 0) {
        return;
    }

    std::unique_lock<std::mutex> lock(monitorMutex);

    auto updatedSnapshot = std::make_shared<ListenerList>();

    if (listenerSnapshot != 0) {
        *updatedSnapshot = *listenerSnapshot;
    }

    updatedSnapshot->push_back(listener);
    listenerSnapshot = updatedSnapshot;
}
