#ifndef STI_UTILS_TASK_H
#define STI_UTILS_TASK_H

#include <mutex>


namespace STI
{
namespace Utils
{

enum class TaskStatus { Active, Inactive };


class Task
{
public:
	
	Task(int id);
	virtual ~Task() {}

	bool operator<(const Task& rhs) const;
	bool operator==(const Task& rhs) const;

	int getID() const;
	bool isActive() const;
	TaskStatus getStatus() const;
	void setStatus(const TaskStatus& newStatus);

	virtual bool isReadyToRun();		//allows for unscheduled task abort

	virtual double secondsToNextRun() const = 0;
	virtual void run() = 0;
	virtual void skipTask() = 0;
	virtual bool repeat() = 0;		//After running, does the task repeat, or is it removed?

private:
	
	int taskID;
	TaskStatus status;

	mutable std::mutex taskMutex;
};


} // UTILS
} // STI


#endif
