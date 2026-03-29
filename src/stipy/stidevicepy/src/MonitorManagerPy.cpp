#include "MonitorManagerPy.h"

#include <sti/device/MonitorListener.h>

using STI::Python::MonitorManagerPy;
using STI::Device::Monitor;
using STI::Device::MonitorManager;
using STI::Device::MonitorListener;
using STI::Device::MonitorStatus;
using STI::Utils::MixedValue;


MonitorManagerPy::MonitorManagerPy(const std::shared_ptr<MonitorManager>& manager)
    : monitorManager(manager)
{
}


std::vector<std::string> MonitorManagerPy::getIDs()
{
    std::vector<std::string> ids;
    MonitorManagerPy::getIDs(ids);
    return ids;
}

std::shared_ptr<Monitor> MonitorManagerPy::getMonitor(const std::string& id)
{
    std::shared_ptr<Monitor> monitor;

    if (MonitorManagerPy::getMonitor(id, monitor)) {
        return monitor;
    }

    return nullptr;
}

std::vector<std::shared_ptr<Monitor>> MonitorManagerPy::getMonitors()
{
    std::vector<std::shared_ptr<Monitor>> monitors;
    MonitorManagerPy::getMonitors(monitors);
    return monitors;
}

MonitorStatus MonitorManagerPy::getStatus(const std::string& id) const
{
    if (monitorManager != 0) {
        return monitorManager->getStatus(id);
    }

    return MonitorStatus::Missing;
}

MixedValue MonitorManagerPy::getValue(const std::string& id)
{
    if (monitorManager != 0) {
        return monitorManager->getValue(id);
    }

    return MixedValue();
}

void MonitorManagerPy::activate(const std::string& id)
{
    if (monitorManager != 0) {
        monitorManager->activate(id);
    }
}

void MonitorManagerPy::deactivate(const std::string& id)
{
    if (monitorManager != 0) {
        monitorManager->deactivate(id);
    }
}

void MonitorManagerPy::activateAll()
{
    if (monitorManager != 0) {
        monitorManager->activateAll();
    }
}

void MonitorManagerPy::deactivateAll()
{
    if (monitorManager != 0) {
        monitorManager->deactivateAll();
    }
}

bool MonitorManagerPy::getIDs(std::vector<std::string>& ids)
{
    if (monitorManager != 0) {
        return monitorManager->getIDs(ids);
    }

    return false;
}

bool MonitorManagerPy::getMonitor(const std::string& id, std::shared_ptr<Monitor>& monitor)
{
    if (monitorManager != 0) {
        return monitorManager->getMonitor(id, monitor);
    }

    return false;
}

bool MonitorManagerPy::getMonitors(std::vector<std::shared_ptr<Monitor>>& monitors)
{
    if (monitorManager != 0) {
        return monitorManager->getMonitors(monitors);
    }

    return false;
}

void MonitorManagerPy::addListener(const std::shared_ptr<MonitorListener>& listener)
{
    if (monitorManager != 0) {
        monitorManager->addListener(listener);
    }
}
