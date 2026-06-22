#include <sti/utils/TaskScheduler.h>
#include <sti/utils/TimeStamp.h>
#include <sti/utils/utils.h>

#include <algorithm>
#include <chrono>

using STI::Utils::TaskScheduler;
using STI::Utils::Task;
using STI::Utils::TaskStatus;
using STI::Utils::TaskSchedulerEvent;
using STI::Utils::TaskSchedulerEventType;


TaskScheduler::TaskScheduler()
: running(false)
{
	setMinSleep(1);		//seconds
}

TaskScheduler::~TaskScheduler()
{
	stop();
}

void TaskScheduler::start()
{
	std::unique_lock<std::mutex> writeLock(schedulerMutex);
	
	if (!running) {
		running = true;
		taskThread = std::thread(&TaskScheduler::taskLoop, this);
	}
}

void TaskScheduler::stop()
{
	{
		std::unique_lock<std::mutex> writeLock(schedulerMutex);

		if (!running) return;

		running = false;
		schedulerCondition.notify_all();
	}

	if (taskThread.joinable()) {
		taskThread.join();
	}
}

void TaskScheduler::addListener(STI::Utils::TaskSchedulerListener* listener)
{
	listeners.push_back(listener);
}

void TaskScheduler::sendEvent(const PendingEvent& event)
{
	for (auto& listener : listeners) {
		if (listener != 0) {
			listener->handleEvent(TaskSchedulerEvent(event.type, event.taskID, event.timestamp));
		}
	}
}

void TaskScheduler::sendEvents(const PendingEvents& events)
{
	for (auto& event : events) {
		sendEvent(event);
	}
}

void TaskScheduler::getIDs(std::set<std::string>& ids) const
{
	tasks.getKeys(ids);
}

bool TaskScheduler::getTask(const std::string& taskID, std::shared_ptr<Task>& task) const
{
	return tasks.get(taskID, task) && task != 0;
}

void TaskScheduler::getTasks(std::vector<std::shared_ptr<Task>>& allTasks) const
{
	tasks.getValues(allTasks);
}

void TaskScheduler::addTask(const std::shared_ptr<Task>& task)
{
	if (task == 0) return;

	PendingEvents events;
	{
		std::unique_lock<std::mutex> writeLock(schedulerMutex);

		tasks.add(task->getID(), task);
		task->setStatus(TaskStatus::Active);
		events.emplace_back(TaskSchedulerEventType::Add, task->getID());

		auto it = findActiveTask(task->getID());
		if (it != activeTasks.end()) {
			//already added; replace
			activeTasks.erase(it);
		}
		activeTasks.push_back(task);
		events.emplace_back(TaskSchedulerEventType::Activate, task->getID());

		schedulerCondition.notify_all();
	}

	sendEvents(events);
}


std::vector<std::shared_ptr<Task>>::iterator TaskScheduler::findActiveTask(const std::string& id)
{
	auto it = find_if(activeTasks.begin(), activeTasks.end(), [&id](const std::shared_ptr<Task>& t) { return t->getID() == id; });
	return it;
}

void TaskScheduler::sortActiveTasks()
{
	std::sort(activeTasks.begin(), activeTasks.end(), STI::Utils::compare_shared_ptr<Task>);
}

void TaskScheduler::removeTask(const std::string& taskID)
{
	PendingEvents events;
	{
		std::unique_lock<std::mutex> writeLock(schedulerMutex);
		removeTask_(taskID, events);
		schedulerCondition.notify_all();
	}

	sendEvents(events);
}

void TaskScheduler::removeTask_(const std::string& taskID, PendingEvents& events)
{
	deactivateTask_(taskID, events);
	tasks.remove(taskID);
	events.emplace_back(TaskSchedulerEventType::Remove, taskID);
}

void TaskScheduler::clear()
{
	PendingEvents events;
	{
		std::unique_lock<std::mutex> writeLock(schedulerMutex);

		std::vector<std::shared_ptr<Task>> allTasks;
		tasks.getValues(allTasks);
		
		for (auto& task : allTasks) {
			if (task != 0) {
				task->setStatus(TaskStatus::Inactive);
			}
		}

		tasks.clear();
		activeTasks.clear();
		events.emplace_back(TaskSchedulerEventType::Refresh, "");

		schedulerCondition.notify_all();
	}

	sendEvents(events);
}

void TaskScheduler::activateTask(const std::string& taskID)
{
	PendingEvents events;
	{
		std::unique_lock<std::mutex> writeLock(schedulerMutex);
		
		std::shared_ptr<Task> task;
		auto it = findActiveTask(taskID);

		if (tasks.get(taskID, task) && task != 0) {
			task->setStatus(TaskStatus::Active);
			events.emplace_back(TaskSchedulerEventType::Activate, taskID);
			
			if (it == activeTasks.end()) {
				activeTasks.push_back(task);
			}
		}
		else {
			//not found in map; should not be in activeTasks either
			if (it != activeTasks.end()) {
				activeTasks.erase(it);
			}
		}

		schedulerCondition.notify_all();
	}

	sendEvents(events);
}

void TaskScheduler::deactivateTask(const std::string& taskID)
{
	PendingEvents events;
	{
		std::unique_lock<std::mutex> writeLock(schedulerMutex);
		deactivateTask_(taskID, events);
		schedulerCondition.notify_all();
	}

	sendEvents(events);
}

void TaskScheduler::deactivateTask_(const std::string& taskID, PendingEvents& events)
{
	std::shared_ptr<Task> task;
	auto it = findActiveTask(taskID);

	if (tasks.get(taskID, task) && task != 0) {
		task->setStatus(TaskStatus::Inactive);
		events.emplace_back(TaskSchedulerEventType::Deactivate, taskID);
	}

	if (it != activeTasks.end()) {
		activeTasks.erase(it);
	}
}

void TaskScheduler::runNow(const std::string& taskID)
{
	PendingEvents events;
	{
		std::unique_lock<std::mutex> taskLock(schedulerMutex);

		std::shared_ptr<Task> task;
		
		if (getTask(taskID, task)) {
			run(task, events);
			schedulerCondition.notify_all();
		}
	}

	sendEvents(events);
}

void TaskScheduler::run(std::shared_ptr<Task>& task, PendingEvents& events)
{
	if (task == 0) return;

	if (task->isReadyToRun()) {
		const auto timestamp = STI::Utils::TimeStamp().toString();
		task->run();
		events.emplace_back(TaskSchedulerEventType::Run, task->getID(), timestamp);
	}
	else {
		task->skipTask();
	}

	if (!task->repeat()) {
		deactivateTask_(task->getID(), events);
	}
}

void TaskScheduler::setMinSleep(double sleep)
{
	std::unique_lock<std::mutex> taskLock(schedulerMutex);
	minSleep = sleep;	//seconds
}

void TaskScheduler::taskLoop()
{
	double maxSleep = 10000;		//seconds
	double coarseSleep = 10;		//boundary (in seconds) between using seconds vs milliseconds to specify sleep
	double nextSleep = maxSleep;	//seconds

	while (true)
	{
		PendingEvents events;
		std::unique_lock<std::mutex> taskLock(schedulerMutex);

		if (!running) {
			break;
		}

		sortActiveTasks();

		std::vector<std::shared_ptr<Task>> readyTasks;
		for (auto& task : activeTasks) {
			if (task != 0 && task->secondsToNextRun() <= 0) {
				readyTasks.push_back(task);
			}
			else {
				break;	//the rest of the tasks have positive waits
			}
		}

		for (auto& task : readyTasks) {
			if (task != 0 && findActiveTask(task->getID()) != activeTasks.end()) {
				run(task, events);
			}
		}

		sortActiveTasks();	//resort so recently run task are at the back

		if (!events.empty()) {
			taskLock.unlock();
			sendEvents(events);
			continue;
		}
		
		if (activeTasks.size() > 0) {
			nextSleep = activeTasks.at(0)->secondsToNextRun();	//first task is the next to run
		}
		else {
			nextSleep = maxSleep;
		}
		
		if (nextSleep > maxSleep) {
			nextSleep = maxSleep;
		}
		else if (nextSleep < minSleep) {
			nextSleep = minSleep;
		}

		//wait
		auto now = std::chrono::system_clock::now();

		if (nextSleep > coarseSleep) {
			//coarse sleep
			schedulerCondition.wait_until(taskLock, now + std::chrono::seconds( static_cast<int>(nextSleep - 0.5 * coarseSleep) ));
		}
		else {
			//fine sleep
			schedulerCondition.wait_until(taskLock, now + std::chrono::milliseconds(static_cast<int>(nextSleep * 1000)));
		}
	}
}
