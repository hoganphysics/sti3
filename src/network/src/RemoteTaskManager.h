#ifndef STI_NETWORK_REMOTETASKMANAGER_H
#define STI_NETWORK_REMOTETASKMANAGER_H

#include "generated/tasks.h"
#include <sti/device/TaskManager.h>
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>
#include <optional>
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

    RemoteTaskManager(::STI::TNetwork::TTaskManager_var manager);
    ~RemoteTaskManager();

    void getTaskIDs(std::set<std::string>& ids) const;

    STI::Utils::TaskStatus getTaskStatus(const std::string& taskID) const;
    std::optional<STI::Utils::TimeStamp> getTaskLastRunTime(const std::string& taskID) const;
    void setStatus(const std::string& taskID, const STI::Utils::TaskStatus& newStatus);

    bool getTask(const std::string& id, std::shared_ptr<STI::Utils::Task>& task) const;
    void getTasks(std::vector<std::shared_ptr<STI::Utils::Task>>& tasks) const;

	void removeTask(const std::string& taskID);
	void clear();

	void activateTask(const std::string& taskID);
	void deactivateTask(const std::string& taskID);

    void runTask(const std::string& taskID);
    double secondsToNextRun(const std::string& taskID);

    bool ping() const;

private:

	mutable std::mutex taskMutex;
};


} //Network
} //STI


#endif
