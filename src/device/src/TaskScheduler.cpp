#include <sti/utils/TaskScheduler.h>
#include <sti/utils/TimeStamp.h>

#include <algorithm>
#include <chrono>

using STI::Utils::TaskScheduler;
using STI::Utils::Task;
using STI::Utils::TaskStatus;
using STI::Utils::TaskSchedulerEvent;
using STI::Utils::TaskSchedulerEventType;

namespace
{

struct ScheduledTask
{
	std::shared_ptr<Task> task;
	double secondsToNextRun;
};

} // namespace


TaskScheduler::TaskScheduler()
: running(false), schedulerRevision(0)
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
		++schedulerRevision;
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

		++schedulerRevision;
		schedulerCondition.notify_all();
	}

	sendEvents(events);
}


std::vector<std::shared_ptr<Task>>::iterator TaskScheduler::findActiveTask(const std::string& id)
{
	auto it = find_if(activeTasks.begin(), activeTasks.end(), [&id](const std::shared_ptr<Task>& t) { return t->getID() == id; });
	return it;
}

bool TaskScheduler::isActiveTask_(const std::string& id)
{
	return findActiveTask(id) != activeTasks.end();
}

void TaskScheduler::removeTask(const std::string& taskID)
{
	PendingEvents events;
	{
		std::unique_lock<std::mutex> writeLock(schedulerMutex);
		removeTask_(taskID, events);
		++schedulerRevision;
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

		++schedulerRevision;
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

		++schedulerRevision;
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
		++schedulerRevision;
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
	std::shared_ptr<Task> task;
	{
		std::unique_lock<std::mutex> taskLock(schedulerMutex);
		tasks.get(taskID, task);
	}

	if (task != 0) {
		run(task, events, false);
	}

	sendEvents(events);
}

bool TaskScheduler::run(const std::shared_ptr<Task>& task, PendingEvents& events, bool requireActive)
{
	if (task == 0) return false;

	const auto taskID = task->getID();

	{
		std::unique_lock<std::mutex> taskLock(schedulerMutex);

		std::shared_ptr<Task> currentTask;
		if (!tasks.get(taskID, currentTask) || currentTask != task) {
			return false;
		}

		if (requireActive && !isActiveTask_(taskID)) {
			return false;
		}

		if (runningTasks.count(taskID) > 0) {
			return false;
		}

		runningTasks.insert(taskID);
		++schedulerRevision;
		schedulerCondition.notify_all();
	}

	std::optional<STI::Utils::TimeStamp> timestamp;
	bool repeats = true;

	try {
		// Task virtuals may enter Python and acquire the GIL, so they must run
		// without schedulerMutex held.
		if (task->isReadyToRun()) {
			timestamp = task->runNow();
		}
		else {
			task->skipTask();
		}

		repeats = task->repeat();
	}
	catch (...) {
		std::unique_lock<std::mutex> taskLock(schedulerMutex);
		runningTasks.erase(taskID);
		++schedulerRevision;
		schedulerCondition.notify_all();
		throw;
	}

	{
		std::unique_lock<std::mutex> taskLock(schedulerMutex);
		runningTasks.erase(taskID);
		++schedulerRevision;

		std::shared_ptr<Task> currentTask;
		if (tasks.get(taskID, currentTask) && currentTask == task) {
			if (timestamp.has_value()) {
				events.emplace_back(TaskSchedulerEventType::Run, taskID, timestamp.value());
			}

			if (!repeats) {
				deactivateTask_(taskID, events);
			}
		}

		schedulerCondition.notify_all();
	}

	return true;
}

void TaskScheduler::setMinSleep(double sleep)
{
	std::unique_lock<std::mutex> taskLock(schedulerMutex);
	minSleep = sleep;	//seconds
	++schedulerRevision;
	schedulerCondition.notify_all();
}

void TaskScheduler::taskLoop()
{
	double maxSleep = 10000;		//seconds
	double coarseSleep = 10;		//boundary (in seconds) between using seconds vs milliseconds to specify sleep
	double nextSleep = maxSleep;	//seconds

	while (true)
	{
		std::vector<std::shared_ptr<Task>> tasksSnapshot;
		std::size_t observedRevision = 0;
		double observedMinSleep = 0;

		{
			std::unique_lock<std::mutex> taskLock(schedulerMutex);

			if (!running) {
				break;
			}

			tasksSnapshot = activeTasks;
			observedRevision = schedulerRevision;
			observedMinSleep = minSleep;
		}

		// Task timing callbacks can be user/Python code; keep them outside the
		// scheduler mutex to avoid lock-order deadlocks with addTask/runTask.
		std::vector<ScheduledTask> scheduledTasks;
		scheduledTasks.reserve(tasksSnapshot.size());

		for (auto& task : tasksSnapshot) {
			if (task != 0) {
				scheduledTasks.push_back(ScheduledTask{task, task->secondsToNextRun()});
			}
		}

		std::sort(scheduledTasks.begin(), scheduledTasks.end(),
			[](const ScheduledTask& lhs, const ScheduledTask& rhs) {
				return lhs.secondsToNextRun < rhs.secondsToNextRun;
			});

		PendingEvents events;
		bool ranTask = false;
		for (auto& scheduledTask : scheduledTasks) {
			if (scheduledTask.secondsToNextRun > 0) {
				break;
			}
			ranTask = run(scheduledTask.task, events, true) || ranTask;
		}

		if (!events.empty()) {
			sendEvents(events);
		}

		if (ranTask) {
			continue;
		}
		
		if (!scheduledTasks.empty()) {
			nextSleep = scheduledTasks.front().secondsToNextRun;
		}
		else {
			nextSleep = maxSleep;
		}
		
		if (nextSleep > maxSleep) {
			nextSleep = maxSleep;
		}
		else if (nextSleep < observedMinSleep) {
			nextSleep = observedMinSleep;
		}

		//wait
		auto now = std::chrono::system_clock::now();
		std::unique_lock<std::mutex> taskLock(schedulerMutex);

		if (!running) {
			break;
		}

		if (schedulerRevision != observedRevision) {
			continue;
		}

		if (nextSleep > coarseSleep) {
			//coarse sleep
			schedulerCondition.wait_until(
				taskLock,
				now + std::chrono::seconds(static_cast<int>(nextSleep - 0.5 * coarseSleep)),
				[this, observedRevision]() { return !running || schedulerRevision != observedRevision; });
		}
		else {
			//fine sleep
			schedulerCondition.wait_until(
				taskLock,
				now + std::chrono::milliseconds(static_cast<int>(nextSleep * 1000)),
				[this, observedRevision]() { return !running || schedulerRevision != observedRevision; });
		}
	}
}
