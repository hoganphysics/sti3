
#include "TTaskManager_i.h"

#include <sti/device/TaskManager.h>
#include <sti/utils/Task.h>

#include "convert/Convert_Task.h"
#include "convert/Convert_EventEngine.h"

using STI::Network::convert;
using STI::TNetwork::TTaskManager_i;
using STI::Utils::Task;
using STI::TNetwork::TTask;
using STI::TNetwork::TTaskStatus;
using STI::Utils::TaskStatus;
using STI::Utils::TimeStamp;
using STI::TNetwork::TTimeStamp;


TTaskManager_i::TTaskManager_i(const std::shared_ptr<STI::Device::Device>& device)
{
    if (device != 0) {
        device->getTaskManager(taskManager);
    }
}

TTaskManager_i::~TTaskManager_i()
{
}

void TTaskManager_i::getTaskIDs(::STI::TNetwork::TStringSeq_out ids)
{
    STI::TNetwork::TStringSeq_var tStringSeq_var(new STI::TNetwork::TStringSeq);
    std::set<std::string> localIDs;

    ids = new STI::TNetwork::TStringSeq();

    if (taskManager != 0) {

        taskManager->getTaskIDs(localIDs);

        std::vector<std::string> idsVec;
        for (auto& id : localIDs) {
            idsVec.push_back(id);      //deep copy, but names localNames is short
        }
        convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(idsVec, tStringSeq_var);

        (*ids) = tStringSeq_var;
    }
}

TTaskStatus TTaskManager_i::getTaskStatus(const char* taskID)
{
    TTaskStatus tStatus = TTaskStatus::TaskMissing;

    if (taskManager != 0) {
        auto status = taskManager->getTaskStatus(taskID);
        tStatus = convert<TaskStatus, TTaskStatus>(status);
    }
    return tStatus;
}

::CORBA::Boolean TTaskManager_i::getTaskLastRunTime(const char* taskID, ::STI::TNetwork::TTimeStamp& lastRunTime)
{
    if (taskManager != 0) {
        auto localLastRunTime = taskManager->getTaskLastRunTime(taskID);
        if (localLastRunTime.has_value()) {
            lastRunTime = convert<TimeStamp, TTimeStamp>(localLastRunTime.value());
            return true;
        }
    }

    lastRunTime = TTimeStamp{};
    return false;
}

void TTaskManager_i::setStatus(const char* taskID, ::STI::TNetwork::TTaskStatus newStatus)
{
    if (taskManager != 0) {
        taskManager->setStatus(taskID, convert<TTaskStatus, TaskStatus>(newStatus));
    }
}

::CORBA::Boolean TTaskManager_i::getTask(const char* taskID, ::STI::TNetwork::TTask_out task)
{
    bool success = false;

    std::shared_ptr<Task> localTask;
    task = new TTask();

    if (taskManager != 0) {
        success = taskManager->getTask(taskID, localTask) && localTask != 0;
    }

    if (success) {
        success = convert<std::shared_ptr<Task>, TTask>(localTask, (TTask&)(*task));
    }

    return success;
}

void TTaskManager_i::getTasks(::STI::TNetwork::TTaskSeq_out tasks)
{
	std::vector<std::shared_ptr<Task>> localTasks;
	tasks = new STI::TNetwork::TTaskSeq();

	if (taskManager != 0) {
		taskManager->getTasks(localTasks);

		STI::TNetwork::TTaskSeq_var tTaskSeq_var(new STI::TNetwork::TTaskSeq);

		convert<std::shared_ptr<Task>, TTask>(localTasks,
			(_CORBA_Unbounded_Sequence<TTask>&) tTaskSeq_var);

		(*tasks) = tTaskSeq_var;
	}
}

void TTaskManager_i::removeTask(const char* taskID)
{
    if (taskManager != 0) {
        taskManager->removeTask(taskID);
    }
}

void TTaskManager_i::clear()
{
    if (taskManager != 0) {
        taskManager->clear();
    }
}

void TTaskManager_i::activateTask(const char* taskID)
{
    if (taskManager != 0) {
        taskManager->activateTask(taskID);
    }
}

void TTaskManager_i::deactivateTask(const char* taskID)
{
    if (taskManager != 0) {
        taskManager->deactivateTask(taskID);
    }
}

void TTaskManager_i::runTask(const char* taskID)
{
    if (taskManager != 0) {
        taskManager->runTask(taskID);
    }
}

::CORBA::Double TTaskManager_i::secondsToNextRun(const char* taskID)
{
    std::shared_ptr<Task> task;
    double result = 0;

    if (taskManager != 0 && taskManager->getTask(taskID, task) && task != 0) {
        result = task->secondsToNextRun();
    }
    return static_cast<::CORBA::Double>(result);
}

::CORBA::Boolean TTaskManager_i::ping()
{
	return true;
}
