#ifndef STI_UTILS_TASK_H
#define STI_UTILS_TASK_H

#include <sti/utils/MixedValue.h>
#include <sti/utils/MetaData.h>

#include <mutex>


namespace STI
{
namespace Utils
{

enum class TaskStatus { Active, Inactive, Missing };

class Task;

class Task
{
public:
	
	Task(const std::string& id);
	Task(const std::string& id, const STI::Utils::MixedValue& metaData);
	virtual ~Task() {}

	bool operator<(const Task& rhs) const;
	bool operator==(const Task& rhs) const;

	std::string getID() const;
	virtual bool isActive() const;
	virtual TaskStatus getStatus() const;
	virtual void setStatus(const TaskStatus& newStatus);

	const STI::Utils::MixedValue& getMetaData() const;
	STI::Utils::MixedValue getMetaData(const std::string& key) const;
	Task& addMetaData(const std::string& key, const STI::Utils::MixedValue& data);

	virtual bool isReadyToRun();		//allows for unscheduled task abort

	virtual double secondsToNextRun() const = 0;
	virtual void run() = 0;
	virtual void skipTask() = 0;
	virtual bool repeat() = 0;		//After running, does the task repeat, or is it removed?

private:
	
	std::string taskID;
	TaskStatus status;
	STI::Utils::MetaData metaData;

	mutable std::mutex taskMutex;
};


} // UTILS
} // STI


#endif
