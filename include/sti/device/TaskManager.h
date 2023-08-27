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

    virtual void getTaskIDs(std::set<std::string>& ids) = 0;
    virtual bool getTask(const std::string& id, std::shared_ptr<STI::Utils::Task>& task) = 0;
    virtual void getTasks(std::vector<std::shared_ptr<STI::Utils::Task>>& tasks) = 0;

	virtual void removeTask(const std::string& taskID) = 0;
	virtual void clear() = 0;

	virtual void activateTask(const std::string& taskID) = 0;
	virtual void deactivateTask(const std::string& taskID) = 0;

};


} //Device
} //STI

#endif
