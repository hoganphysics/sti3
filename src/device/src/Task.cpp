#include <sti/utils/Task.h>

using STI::Utils::Task;
using STI::Utils::TaskStatus;


Task::Task(const std::string& id)
: Task(id, STI::Utils::MixedValue())
{
}

Task::Task(const std::string& id, const STI::Utils::MixedValue& data)
: taskID(id), status(TaskStatus::Active), metaData(data)
{
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

STI::Utils::TimeStamp Task::runNow()
{
	STI::Utils::TimeStamp runTime;
	run();
	setLastRunTime(runTime);
	return runTime;
}

bool Task::hasLastRunTime() const
{
	std::unique_lock<std::mutex> taskLock(taskMutex);
	return lastRunTime.has_value();
}

std::optional<STI::Utils::TimeStamp> Task::getLastRunTime() const
{
	std::unique_lock<std::mutex> taskLock(taskMutex);
	return lastRunTime;
}

void Task::setLastRunTime(const STI::Utils::TimeStamp& runTime)
{
	std::unique_lock<std::mutex> taskLock(taskMutex);
	lastRunTime = runTime;
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
