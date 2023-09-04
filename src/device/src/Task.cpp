
#include <sti/utils/Task.h>

#include <chrono>

using STI::Utils::Task;
using STI::Utils::TaskStatus;


Task::Task(const std::string& id)
: Task(id, STI::Utils::MixedValue())
{
}

Task::Task(const std::string& id, const STI::Utils::MixedValue& data)
: taskID(id), status(TaskStatus::Active), metaData(data)
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

const STI::Utils::MixedValue& Task::getMetaData() const
{
    std::unique_lock<std::mutex> taskLock(taskMutex);
    return metaData.getMetaData();
}

STI::Utils::MixedValue Task::getMetaData(const std::string& key) const
{
    std::unique_lock<std::mutex> taskLock(taskMutex);
    return metaData.getMetaData(key);
}

Task& Task::addMetaData(const std::string& key, const STI::Utils::MixedValue& data)
{
    std::unique_lock<std::mutex> taskLock(taskMutex);

    metaData.addMetaData(key, data);

    return (*this);
}

bool Task::isReadyToRun()
{
	return true;
}
