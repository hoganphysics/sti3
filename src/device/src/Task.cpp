
#include <sti/utils/Task.h>

#include <chrono>

using STI::Utils::Task;
using STI::Utils::TaskStatus;

Task::Task(const std::string& id)
: taskID(id), status(TaskStatus::Active)
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
}

bool Task::operator<(const Task& rhs) const
{ 
	return secondsToNextRun() < rhs.secondsToNextRun();
}

bool Task::operator==(const Task& rhs) const
{
	return secondsToNextRun() == rhs.secondsToNextRun();
}

std::string Task::getID() const
{
	return taskID;
}

bool Task::isActive() const
{
	std::unique_lock<std::mutex> statusLock(taskMutex);
	return status == TaskStatus::Active;
}

TaskStatus Task::getStatus() const
{
	std::unique_lock<std::mutex> statusLock(taskMutex);
	return status;
}

void Task::setStatus(const TaskStatus& newStatus)
{
	std::unique_lock<std::mutex> statusLock(taskMutex);
	status = newStatus;
}

bool Task::isReadyToRun()
{
	return true;
}
