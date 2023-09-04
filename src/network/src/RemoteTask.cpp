
#include "RemoteTask.h"
#include "RemoteTaskManager.h"

#include <sti/utils/Task.h>

using STI::Network::RemoteTask;
using STI::Utils::TaskStatus;


RemoteTask::RemoteTask(const std::string& id, const STI::Utils::MixedValue& metaData)
: Task(id, metaData)
{
}

RemoteTask::~RemoteTask()
{
}

void RemoteTask::attachManager(RemoteTaskManager* manager)
{
    remoteManager = manager;
}


bool RemoteTask::isActive() const
{
    if (remoteManager == 0) return false;
    return remoteManager->getTaskStatus(getID()) == TaskStatus::Active;
}

STI::Utils::TaskStatus RemoteTask::getStatus() const
{
    if (remoteManager == 0) return TaskStatus::Missing;
    return remoteManager->getTaskStatus(getID());
}


void RemoteTask::setStatus(const STI::Utils::TaskStatus& newStatus)
{
    if (remoteManager == 0) return;
    remoteManager->setStatus(getID(), newStatus);
}

void RemoteTask::run()
{
    if (remoteManager == 0) return;

    remoteManager->runTask(getID());
}