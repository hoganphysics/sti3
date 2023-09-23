#include "JTaskManager.h"

using STI::Device::JTaskManager;


JTaskManager::JTaskManager(const std::shared_ptr<STI::Device::TaskManager>& manager)
: taskManager(manager)
{
}

JTaskManager::~JTaskManager()
{
}

std::set<std::string> JTaskManager::getTaskIDs() const
{
    std::set<std::string> ids;
    if (taskManager != 0) {
        taskManager->getTaskIDs(ids);
    }
    return ids;
}

STI::Utils::TaskStatus JTaskManager::getTaskStatus(const std::string& taskID) const
{
    STI::Utils::TaskStatus status = STI::Utils::TaskStatus::Missing;
    if (taskManager != 0) {
        status = taskManager->getTaskStatus(taskID);
    }
    return status;
}

void JTaskManager::setStatus(const std::string& taskID, const STI::Utils::TaskStatus& newStatus)
{
    if (taskManager != 0) {
        taskManager->setStatus(taskID, newStatus);
    }
}


std::shared_ptr<STI::Utils::Task> JTaskManager::getTask(const std::string& taskID) const
{
    std::shared_ptr<STI::Utils::Task> task;
    if (taskManager != 0) {
        taskManager->getTask(taskID, task);
    }
    return task;
}

std::vector<std::shared_ptr<STI::Utils::Task>> JTaskManager::getTasks() const
{
    std::vector<std::shared_ptr<STI::Utils::Task>> tasks;
    if (taskManager != 0) {
        taskManager->getTasks(tasks);
    }
    return tasks;
}


void JTaskManager::removeTask(const std::string& taskID)
{
    if (taskManager != 0) {
        taskManager->removeTask(taskID);
    }
}

void JTaskManager::clear()
{
    if (taskManager != 0) {
        taskManager->clear();
    }
}

void JTaskManager::activateTask(const std::string& taskID)
{
    if (taskManager != 0) {
        taskManager->activateTask(taskID);
    }
}

void JTaskManager::deactivateTask(const std::string& taskID)
{
    if (taskManager != 0) {
        taskManager->deactivateTask(taskID);
    }
}


void JTaskManager::runTask(const std::string& taskID)
{
    if (taskManager != 0) {
        taskManager->runTask(taskID);
    }
}
