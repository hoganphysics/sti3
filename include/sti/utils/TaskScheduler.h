#ifndef STI_UTILS_TASKSCEDULER_H
#define STI_UTILS_TASKSCEDULER_H

#include <sti/utils/Task.h>
#include <sti/utils/SynchronizedMap.h>

#include <vector>
#include <map>
#include <set>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>


namespace STI
{
namespace Utils
{

class TaskSchedulerListener;
class TaskSchedulerEvent;

enum class TaskSchedulerEventType { Add, Remove, Activate, Deactivate, Refresh, Run };


class TaskScheduler
{
public:
	
	TaskScheduler();
	~TaskScheduler();

	void start();
	void stop();

	void addTask(const std::shared_ptr<Task>& task);
	void removeTask(const std::string& taskID);
	void clear();

	void activateTask(const std::string& taskID);
	void deactivateTask(const std::string& taskID);

	void addListener(TaskSchedulerListener* listener);

	void getIDs(std::set<std::string>& ids) const;
	bool getTask(const std::string& taskID, std::shared_ptr<Task>& task) const;
	void getTasks(std::vector<std::shared_ptr<Task>>& tasks) const;

	void runNow(const std::string& taskID);

	void setMinSleep(double sleep);		//in seconds

private:
	struct PendingEvent
	{
		PendingEvent(const TaskSchedulerEventType& type, const std::string& taskID)
		: PendingEvent(type, taskID, std::optional<STI::Utils::TimeStamp>()) {}
		PendingEvent(const TaskSchedulerEventType& type, const std::string& taskID,
			const STI::Utils::TimeStamp& timestamp)
		: PendingEvent(type, taskID, std::optional<STI::Utils::TimeStamp>(timestamp)) {}
		PendingEvent(const TaskSchedulerEventType& type, const std::string& taskID,
			const std::optional<STI::Utils::TimeStamp>& timestamp)
		: type(type), taskID(taskID), timestamp(timestamp) {}

		TaskSchedulerEventType type;
		std::string taskID;
		std::optional<STI::Utils::TimeStamp> timestamp;
	};

	using PendingEvents = std::vector<PendingEvent>;
	
	void run(std::shared_ptr<Task>& task, PendingEvents& events);
	
	void removeTask_(const std::string& taskID, PendingEvents& events);
	void deactivateTask_(const std::string& taskID, PendingEvents& events);

	void taskLoop();
	
	void sortActiveTasks();
	std::vector<std::shared_ptr<Task>>::iterator findActiveTask(const std::string& id);

	STI::Utils::SynchronizedMap<std::string, std::shared_ptr<Task>> tasks;
	std::vector<std::shared_ptr<Task>> activeTasks;

	void sendEvent(const PendingEvent& event);
	void sendEvents(const PendingEvents& events);
	std::vector<TaskSchedulerListener*> listeners;

	std::thread taskThread;
	bool running;

	double minSleep;

	mutable std::mutex schedulerMutex;
	mutable std::condition_variable schedulerCondition;
};

class TaskSchedulerEvent
{
public:

	TaskSchedulerEvent(const TaskSchedulerEventType& type, const std::string& taskID)
	: TaskSchedulerEvent(type, taskID, std::optional<STI::Utils::TimeStamp>()) {}
	TaskSchedulerEvent(const TaskSchedulerEventType& type, const std::string& taskID,
		const STI::Utils::TimeStamp& timestamp)
	: TaskSchedulerEvent(type, taskID, std::optional<STI::Utils::TimeStamp>(timestamp)) {}
	TaskSchedulerEvent(const TaskSchedulerEventType& type, const std::string& taskID,
		const std::optional<STI::Utils::TimeStamp>& timestamp)
	: type(type), taskID(taskID), timestamp(timestamp) {}
	virtual ~TaskSchedulerEvent() {}

	TaskSchedulerEventType type;
	std::string taskID;
	std::optional<STI::Utils::TimeStamp> timestamp;
};

class TaskSchedulerListener
{
public:
	virtual ~TaskSchedulerListener() {}

	virtual void handleEvent(const TaskSchedulerEvent& evt) = 0;
};



} // UTILS
} // STI


#endif
