
#include "RemoteTask.h"
#include "RemoteTaskManager.h"

#include <sti/utils/Task.h>

#include <optional>

using STI::Network::RemoteTask;
using STI::Utils::TaskStatus;


RemoteTask::RemoteTask(const std::string& id, const STI::Utils::MixedValue& metaData)
: RemoteTask(id, metaData, std::nullopt)
{
}

RemoteTask::RemoteTask(const std::string& id, const STI::Utils::MixedValue& metaData,
    const std::optional<STI::Utils::TimeStamp>& snapshotLastRunTime)
: Task(id, metaData), remoteManager(nullptr), snapshotLastRunTime(snapshotLastRunTime)
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

bool RemoteTask::hasLastRunTime() const
{
    return getLastRunTime().has_value();
}

std::optional<STI::Utils::TimeStamp> RemoteTask::getLastRunTime() const
{
    if (remoteManager == 0) return snapshotLastRunTime;
    return remoteManager->getTaskLastRunTime(getID());
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
