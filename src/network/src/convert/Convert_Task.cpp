
#include "Convert_Task.h"
#include "RemoteTask.h"


using STI::Network::convert;
using STI::Utils::Task;
using STI::TNetwork::TTask;
using STI::Utils::TaskStatus;
using STI::TNetwork::TTaskStatus;
using STI::Network::RemoteTask;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;


template<>
std::shared_ptr<Task> STI::Network::convert<TTask, std::shared_ptr<Task>>(const TTask& tTask)
{
    std::shared_ptr<Task> task;
    convert<TTask, std::shared_ptr<Task>>(tTask, task);
    return task;
}

template<>
TTask STI::Network::convert<std::shared_ptr<Task>, TTask>(const std::shared_ptr<Task>& task)
{
    TTask tTask;
    convert<std::shared_ptr<Task>, TTask>(task, tTask);
    return tTask;
}


template<>
bool STI::Network::convert<TTask, std::shared_ptr<Task>>(const TTask& tTask, std::shared_ptr<Task>& task)
{
    MixedValue metaData = convert<TMixedValue, MixedValue>(tTask.metaData);
    auto status = convert<TTaskStatus, TaskStatus>(tTask.status);

    task = std::make_shared<RemoteTask>(convert<::CORBA::String_member, std::string>(tTask.taskID), metaData);
    // task->setStatus(status);
    return true;
}

template<>
bool STI::Network::convert<std::shared_ptr<Task>, TTask>(const std::shared_ptr<Task>& task, TTask& tTask)
{
    if (task == 0) return false;

    tTask.taskID = convert<std::string, ::CORBA::String_member>(task->getID());
    tTask.status = convert<TaskStatus, TTaskStatus>(task->getStatus());
    tTask.metaData = convert<MixedValue, TMixedValue>(task->getMetaData());

    return true;
}


//TaskStatus
template<>
TaskStatus STI::Network::convert<TTaskStatus, TaskStatus>(const TTaskStatus& tStatus)
{
    //enum class TaskStatus { Active, Inactive };
    //TTaskStatus { TaskActive, TaskInactive }

    TaskStatus status;

    switch (tStatus) 
    {
    case TTaskStatus::TaskActive:
        status = TaskStatus::Active;
        break;
    case TTaskStatus::TaskInactive:
        status = TaskStatus::Inactive;
        break;
    case TTaskStatus::TaskMissing:
        status = TaskStatus::Missing;
        break;
    default:
        status = TaskStatus::Missing;
        break;
    }
    return status;
}

template<>
TTaskStatus STI::Network::convert<TaskStatus, TTaskStatus>(const TaskStatus& status)
{
    TTaskStatus tStatus;

    switch (status) 
    {
    case TaskStatus::Active:
        tStatus = TTaskStatus::TaskActive;
        break;
    case TaskStatus::Inactive:
        tStatus = TTaskStatus::TaskInactive;
        break;
    case TaskStatus::Missing:
        tStatus = TTaskStatus::TaskMissing;
        break;
    default:
        tStatus = TTaskStatus::TaskMissing;
        break;
    }
    return tStatus;
}
