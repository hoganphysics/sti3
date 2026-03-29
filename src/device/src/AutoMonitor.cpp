#include <sti/device/AutoMonitor.h>
#include <sti/utils/IntervalTask.h>

#include "LocalTaskManager.h"

#include <stdexcept>

using STI::Device::AutoMonitor;
using STI::Device::LocalTaskManager;
using STI::Device::MonitorStatus;
using STI::Utils::IntervalTask;


std::shared_ptr<AutoMonitor> AutoMonitor::create(
    const std::string& id,
    double updateInterval_s,
    const Updater& updater,
    const std::shared_ptr<LocalTaskManager>& taskManager)
{
    auto monitor = std::shared_ptr<AutoMonitor>(new AutoMonitor(id, updateInterval_s, updater, taskManager));
    monitor->initializeTask();
    return monitor;
}

AutoMonitor::AutoMonitor(
    const std::string& id,
    double updateInterval_s,
    const Updater& updater,
    const std::shared_ptr<LocalTaskManager>& taskManager)
    : LocalMonitor(id),
      updateInterval_s(updateInterval_s),
      updater(updater),
      taskManager(taskManager),
      taskID(makeTaskID())
{
    if (!this->updater) {
        throw std::invalid_argument("AutoMonitor requires a valid updater function.");
    }

    if (taskManager == 0) {
        throw std::invalid_argument("AutoMonitor requires a valid LocalTaskManager.");
    }
}

AutoMonitor::~AutoMonitor()
{
    if (auto manager = taskManager.lock()) {
        manager->removeTask(taskID);
    }
}

void AutoMonitor::activate()
{
    LocalMonitor::activate();

    if (auto manager = taskManager.lock()) {
        manager->activateTask(taskID);
    }
}

void AutoMonitor::deactivate()
{
    LocalMonitor::deactivate();

    if (auto manager = taskManager.lock()) {
        manager->deactivateTask(taskID);
    }
}

void AutoMonitor::initializeTask()
{
    auto manager = taskManager.lock();

    if (manager == 0) {
        return;
    }

    auto updateTask = std::make_shared<IntervalTask>(
        taskID,
        updateInterval_s,
        [weakSelf = weak_from_this()]() {
            if (auto self = weakSelf.lock()) {
                if (self->getStatus() == MonitorStatus::Active) {
                    self->setValue(self->updater());
                }
            }
        });

    manager->addTask(updateTask);

    if (getStatus() != MonitorStatus::Active) {
        manager->deactivateTask(taskID);
    }
}

std::string AutoMonitor::makeTaskID() const
{
    return "Monitor:" + getID() + ":AutoUpdate";
}
