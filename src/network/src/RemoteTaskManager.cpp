#include "RemoteTaskManager.h"
#include "RemoteTask.h"


#include "convert/Convert_Task.h"

using STI::Network::RemoteTaskManager;

using STI::Network::convert;
using STI::Utils::Task;
using STI::TNetwork::TTask;
using STI::TNetwork::TTaskStatus;
using STI::Utils::TaskStatus;
using STI::Network::RemoteTask;



RemoteTaskManager::RemoteTaskManager(::STI::TNetwork::TTaskManager_var manager)
: TReferenceHolder<::STI::TNetwork::TTaskManager>(manager)
{
}

RemoteTaskManager::~RemoteTaskManager()
{
}


void RemoteTaskManager::getTaskIDs(std::set<std::string>& ids) const
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return;

	STI::TNetwork::TStringSeq_var tIDs(new STI::TNetwork::TStringSeq);

	ids.clear();

	try {
		getTRef()->getTaskIDs(tIDs);	//remote call
		
		std::vector<std::string> idsVec;
		convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tIDs, idsVec);		//only vector<string> is available
		
		ids.insert(idsVec.begin(), idsVec.end());		//deep copy, but ids should be very short
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}

TaskStatus RemoteTaskManager::getTaskStatus(const std::string& taskID) const
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

    TaskStatus status = TaskStatus::Missing;

	if (isDisabled()) return status;

	try {
		auto tStatus = getTRef()->getTaskStatus(
				convert<std::string, ::CORBA::String_member>(taskID));	//remote call
        status = convert<TTaskStatus, TaskStatus>(tStatus);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
    return status;
}

void RemoteTaskManager::setStatus(const std::string& taskID, const STI::Utils::TaskStatus& newStatus)
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return;

	try {
		getTRef()->setStatus(
				convert<std::string, ::CORBA::String_member>(taskID), 
				convert<TaskStatus, TTaskStatus>(newStatus));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}


bool RemoteTaskManager::getTask(const std::string& id, std::shared_ptr<Task>& task) const
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TTask_var tTask(new STI::TNetwork::TTask);

	bool success = false;

	try {
		success = getTRef()->getTask(convert<std::string, ::CORBA::String_member>(id), tTask);	//remote call

		auto success = convert<TTask, std::shared_ptr<Task>>(tTask, task);

        if (success) {
            auto remoteTask = std::dynamic_pointer_cast<RemoteTask>(task);
            if (remoteTask) {
                remoteTask->attachManager(const_cast<RemoteTaskManager*>(this)); //because function is const, but RemoteTask needs a link
            }
        }

	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success && (task != 0);
}

void RemoteTaskManager::getTasks(std::vector<std::shared_ptr<STI::Utils::Task>>& tasks) const
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return;
	
	STI::TNetwork::TTaskSeq_var tTasks(new STI::TNetwork::TTaskSeq);

	try {
		getTRef()->getTasks(tTasks);	//remote call

		convert<TTask, std::shared_ptr<STI::Utils::Task>>(tTasks, tasks);

        for (auto& task : tasks) {
            auto remoteTask = std::dynamic_pointer_cast<RemoteTask>(task);
            if (remoteTask) {
                remoteTask->attachManager(const_cast<RemoteTaskManager*>(this));	//because function is const, but RemoteTask needs a link
            }
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


void RemoteTaskManager::removeTask(const std::string& taskID)
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return;

	try {
		getTRef()->removeTask(convert<std::string, ::CORBA::String_member>(taskID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}

void RemoteTaskManager::clear()
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return;

	try {
		getTRef()->clear();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}


void RemoteTaskManager::activateTask(const std::string& taskID)
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return;

	try {
		getTRef()->activateTask(convert<std::string, ::CORBA::String_member>(taskID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}

void RemoteTaskManager::deactivateTask(const std::string& taskID)
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return;

	try {
		getTRef()->deactivateTask(convert<std::string, ::CORBA::String_member>(taskID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}

void RemoteTaskManager::runTask(const std::string& taskID)
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return;

	try {
		getTRef()->runTask(convert<std::string, ::CORBA::String_member>(taskID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}

double RemoteTaskManager::secondsToNextRun(const std::string& taskID)
{
	std::unique_lock<std::mutex> taskLock(taskMutex);
	double result = 0;

	if (isDisabled()) return result;

	try {
		result = getTRef()->secondsToNextRun(convert<std::string, ::CORBA::String_member>(taskID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
	return result;
}

bool RemoteTaskManager::ping() const
{
	std::unique_lock<std::mutex> taskLock(taskMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}
