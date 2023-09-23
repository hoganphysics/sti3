
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

double RemoteTask::secondsToNextRun() const
{
    if (remoteManager == 0) return 0;
    return remoteManager->secondsToNextRun(getID());
}

void RemoteTask::run()
{
    if (remoteManager == 0) return;

    remoteManager->runTask(getID());
}