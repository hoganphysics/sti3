#ifndef STI_UTILS_TASKSCEDULER_H
#define STI_UTILS_TASKSCEDULER_H

#include <sti/utils/Task.h>
#include <sti/utils/SynchronizedMap.h>

#include <vector>
#include <map>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>


namespace STI
{
namespace Utils
{


class TaskScheduler
{
public:
	
	TaskScheduler();
	~TaskScheduler();

	void start();
	void stop();

	void addTask(const std::shared_ptr<Task>& task);
	void removeTask(int taskID);
	void clear();

	void activateTask(int taskID);
	void deactivateTask(int taskID);

	void refresh();

private:

	void removeTask_(int taskID);
	void deactivateTask_(int taskID);

	void taskLoop();
	
	void sortActiveTasks();
	std::vector<std::shared_ptr<Task>>::iterator findActiveTask(int id);

	STI::Utils::SynchronizedMap<int, std::shared_ptr<Task>> tasks;
	std::vector<std::shared_ptr<Task>> activeTasks;

	std::thread taskThread;
	bool running;

	mutable std::mutex schedulerMutex;
	mutable std::condition_variable schedulerCondition;

};


} // UTILS
} // STI


#endif
