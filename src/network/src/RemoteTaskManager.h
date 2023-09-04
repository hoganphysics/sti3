#ifndef STI_NETWORK_REMOTETASKMANAGER_H
#define STI_NETWORK_REMOTETASKMANAGER_H

#include "generated/tasks.h"
#include <sti/device/TaskManager.h>
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>
#include <string>
#include <set>


namespace STI
{
namespace Network
{

class RemoteTaskManager : public STI::Device::TaskManager,
					      public STI::TNetwork::TReferenceHolder<STI::TNetwork::TTaskManager>	//mixin
{
public:

    RemoteTaskManager(::STI::TNetwork::TTaskManager_ptr manager);
    ~RemoteTaskManager();

    void getTaskIDs(std::set<std::string>& ids);

    STI::Utils::TaskStatus getTaskStatus(const std::string& taskID);

    bool getTask(const std::string& id, std::shared_ptr<STI::Utils::Task>& task);
    void getTasks(std::vector<std::shared_ptr<STI::Utils::Task>>& tasks);

	void removeTask(const std::string& taskID);
	void clear();

	void activateTask(const std::string& taskID);
	void deactivateTask(const std::string& taskID);

    void runTask(const std::string& taskID);

    bool ping() const;

    void setStatus(const std::string& taskID, const STI::Utils::TaskStatus& newStatus);

private:

	mutable std::mutex taskMutex;
};


} //Network
} //STI


#endif
