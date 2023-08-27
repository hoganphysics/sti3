
#include "LocalTaskManager.h"

#include <sti/utils/Task.h>
#include <sti/utils/IntervalTask.h>

#include <iostream>

using STI::Device::LocalTaskManager;
using STI::Utils::Task;


LocalTaskManager::LocalTaskManager()
{
    
	// auto task1 = std::make_shared<STI::Utils::IntervalTask>("0", "00:00:02", //"00:00:02"
	// [](){
	// 	std::cout << "Task LocalTaskManager" << std::endl; 
	// });

    // taskScheduler.addTask(task1);
    taskScheduler.start();
}

LocalTaskManager::~LocalTaskManager()
{
    taskScheduler.stop();
    clear();
}

void LocalTaskManager::getTaskIDs(std::set<std::string>& ids)
{
    taskScheduler.getIDs(ids);
}

bool LocalTaskManager::getTask(const std::string& id, std::shared_ptr<STI::Utils::Task>& task)
{
    return taskScheduler.getTask(id, task);
}

void LocalTaskManager::getTasks(std::vector<std::shared_ptr<STI::Utils::Task>>& tasks)
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

std::string LocalTaskManager::getFilename()
{
    return "tasks.ini";
}

// void LocalTaskManager::setLoadFilename(const std::string& filename)
// {
    
// }

void LocalTaskManager::setPersistenceCallback(const std::function<void(void)>& refresher)
{
    
}

bool LocalTaskManager::save(const std::string& filename)
{
    return false;
}

void LocalTaskManager::load(const std::string& filename)
{

}

