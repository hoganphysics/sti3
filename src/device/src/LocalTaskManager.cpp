#include "LocalTaskManager.h"

#include <sti/utils/Task.h>
#include <sti/utils/IntervalTask.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

using STI::Device::LocalTaskManager;
using STI::Utils::Task;
using STI::Utils::TaskStatus;


LocalTaskManager::LocalTaskManager()
{
    persistenceRefresher = [](){};
    
    taskScheduler.start();
    taskScheduler.addListener(this);
}

LocalTaskManager::~LocalTaskManager()
{
    taskScheduler.stop();
    clear();
}

void LocalTaskManager::getTaskIDs(std::set<std::string>& ids) const
{
    taskScheduler.getIDs(ids);
}

TaskStatus LocalTaskManager::getTaskStatus(const std::string& taskID) const
{
    std::shared_ptr<Task> task;
    if (getTask(taskID, task)) {
        return task->getStatus();
    }
    return TaskStatus::Missing;
}

void LocalTaskManager::setStatus(const std::string& taskID, const STI::Utils::TaskStatus& newStatus)
{
    switch (newStatus)
    {
    case TaskStatus::Active:
        activateTask(taskID);
        break;
    case TaskStatus::Inactive:
        deactivateTask(taskID);
        break;
    default:
        break;
    }
}

bool LocalTaskManager::getTask(const std::string& taskID, std::shared_ptr<Task>& task) const
{
    return taskScheduler.getTask(taskID, task);
}

void LocalTaskManager::getTasks(std::vector<std::shared_ptr<Task>>& tasks) const
{
    taskScheduler.getTasks(tasks);
}


void LocalTaskManager::addTask(const std::shared_ptr<Task>& task)
{
    taskScheduler.addTask(task);
}

void LocalTaskManager::removeTask(const std::string& taskID)
{
    taskScheduler.removeTask(taskID);
}

void LocalTaskManager::clear()
{
    taskScheduler.clear();
}


void LocalTaskManager::activateTask(const std::string& taskID)
{
    taskScheduler.activateTask(taskID);
}

void LocalTaskManager::deactivateTask(const std::string& taskID)
{
    taskScheduler.deactivateTask(taskID);
}

/// Run task immediately
void LocalTaskManager::runTask(const std::string& taskID)
{
    taskScheduler.runNow(taskID);
}

std::string LocalTaskManager::getFilename()
{
    return "tasks.xml";
}


void LocalTaskManager::setPersistenceCallback(const std::function<void(void)>& refresher)
{
    persistenceRefresher = refresher;
}

bool LocalTaskManager::save(const std::string& filename)
{
	std::ofstream file( filename );
    cereal::XMLOutputArchive archive( file );

	LocalTaskManager::StoredTasks storedTasks;
    std::vector<std::shared_ptr<Task>> tasks;

    taskScheduler.getTasks(tasks);

    for (auto& task : tasks) {
        if (task == 0) continue;

        LocalTaskManager::StoredTask storedTask;
        storedTask.taskID = task->getID();
        storedTask.status = task->getStatus();

        storedTasks.tasks.push_back(storedTask);
    }

    archive(storedTasks);   //save to disk

	return true;
}

void LocalTaskManager::load(const std::string& filename)
{
	fs::path profilePath = filename;
	if (!fs::exists(profilePath)) return;

	std::ifstream file( filename );
    cereal::XMLInputArchive archive( file );

	LocalTaskManager::StoredTasks storedTasks;
    archive(storedTasks);   //load from disk

	for (auto& storedTask : storedTasks.tasks) {
        if (storedTask.status == TaskStatus::Active) {
            taskScheduler.activateTask(storedTask.taskID);
        }
        else {
            taskScheduler.deactivateTask(storedTask.taskID);
        }
	}
}

void LocalTaskManager::handleEvent(const STI::Utils::TaskSchedulerEvent& evt)
{
    using STI::Utils::TaskSchedulerEventType;
    
    auto taskID = evt.taskID;

    switch (evt.type)
    {
    case TaskSchedulerEventType::Add:
        break;
    case TaskSchedulerEventType::Remove:
        break;
    case TaskSchedulerEventType::Activate:
        break;
    case TaskSchedulerEventType::Deactivate:
        break;
    case TaskSchedulerEventType::Refresh:
        break;
    default:
        break;
    }

    //Catch sequential calls and call once at the end of the wait period
    barrier.wait(std::chrono::milliseconds(1000), persistenceRefresher);

}

//******** Persistence ***********//
// Store taskID and task status so tasks can be put in correct state on restart.

template<class Archive>
void LocalTaskManager::StoredTask::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("taskID", taskID), 
		cereal::make_nvp("status", status)
		);
}

template void LocalTaskManager::StoredTask::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void LocalTaskManager::StoredTask::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template<class Archive>
void LocalTaskManager::StoredTasks::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("tasks", tasks)
		);
}

template void LocalTaskManager::StoredTasks::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void LocalTaskManager::StoredTasks::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
