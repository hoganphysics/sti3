#ifndef STI_DEVICE_TASKMANAGER_H
#define STI_DEVICE_TASKMANAGER_H

#include <sti/utils/Task.h>

#include <vector>
#include <string>
#include <set>
#include <memory>


namespace STI
{
namespace Device
{


class TaskManager
{
public:

    virtual ~TaskManager() {}

    virtual void getTaskIDs(std::set<std::string>& ids) const = 0;
    
    virtual STI::Utils::TaskStatus getTaskStatus(const std::string& taskID) const= 0;
    virtual void setStatus(const std::string& taskID, const STI::Utils::TaskStatus& newStatus) = 0;

    virtual bool getTask(const std::string& taskID, std::shared_ptr<STI::Utils::Task>& task) const= 0;
    virtual void getTasks(std::vector<std::shared_ptr<STI::Utils::Task>>& tasks) const = 0;

	virtual void removeTask(const std::string& taskID) = 0;
	virtual void clear() = 0;

	virtual void activateTask(const std::string& taskID) = 0;
	virtual void deactivateTask(const std::string& taskID) = 0;

    virtual void runTask(const std::string& taskID) = 0;
};


} //Device
} //STI

#endif
