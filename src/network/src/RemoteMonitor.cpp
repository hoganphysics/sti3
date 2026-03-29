#include "RemoteMonitor.h"

#include "RemoteMonitorManager.h"

#include <sti/device/MonitorListener.h>

#include <utility>

using STI::Network::RemoteMonitor;
using STI::Network::RemoteMonitorManager;
using STI::Device::MonitorListener;
using STI::Device::MonitorStatus;
using STI::Utils::MixedValue;

namespace
{

class FilteredMonitorListener : public MonitorListener
{
public:
    FilteredMonitorListener(std::string id, const std::shared_ptr<MonitorListener>& listener)
        : id(std::move(id)), listener(listener)
    {
    }

    void activated(const std::string& updatedID) override
    {
        if (listener != 0 && updatedID == id) {
            listener->activated(updatedID);
        }
    }

    void deactivated(const std::string& updatedID) override
    {
        if (listener != 0 && updatedID == id) {
            listener->deactivated(updatedID);
        }
    }

    void valueUpdated(const std::string& updatedID, const MixedValue& value) override
    {
        if (listener != 0 && updatedID == id) {
            listener->valueUpdated(updatedID, value);
        }
    }

private:
    std::string id;
    std::shared_ptr<MonitorListener> listener;
};

} // namespace


RemoteMonitor::RemoteMonitor(const std::string& id, const std::string& group,
                             MonitorStatus status, const MixedValue& value,
                             const MixedValue& metaData)
    : id_(id),
      group_(group),
      status_(status),
      value_(value),
      metaData_(metaData),
      remoteManager(nullptr)
{
}

RemoteMonitor::~RemoteMonitor()
{
}

void RemoteMonitor::attachManager(RemoteMonitorManager* manager)
{
    remoteManager = manager;
}

std::string RemoteMonitor::getID() const
{
    return id_;
}

std::string RemoteMonitor::getGroup() const
{
    return group_;
}

MonitorStatus RemoteMonitor::getStatus() const
{
    if (remoteManager != nullptr) {
        return remoteManager->getStatus(id_);
    }

    return status_;
}

void RemoteMonitor::activate()
{
    if (remoteManager != nullptr) {
        remoteManager->activate(id_);
    }
}

void RemoteMonitor::deactivate()
{
    if (remoteManager != nullptr) {
        remoteManager->deactivate(id_);
    }
}

MixedValue RemoteMonitor::getValue()
{
    if (remoteManager != nullptr) {
        value_ = remoteManager->getUpdatedValue(id_);
    }

    return value_;
}

const MixedValue& RemoteMonitor::getMetaData() const
{
    return metaData_.getMetaData();
}

MixedValue RemoteMonitor::getMetaData(const std::string& key) const
{
    return metaData_.getMetaData(key);
}

void RemoteMonitor::addListener(const std::shared_ptr<MonitorListener>& listener)
{
    if (remoteManager == nullptr || listener == 0) {
        return;
    }

    remoteManager->addListener(std::make_shared<FilteredMonitorListener>(id_, listener));
}

MonitorStatus RemoteMonitor::getStoredStatus() const
{
    return status_;
}

const MixedValue& RemoteMonitor::getStoredValue() const
{
    return value_;
}
