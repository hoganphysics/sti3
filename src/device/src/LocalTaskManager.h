#ifndef STI_DEVICE_LOCALTASKMANAGER_H
#define STI_DEVICE_LOCALTASKMANAGER_H

#include <sti/device/TaskManager.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/TaskScheduler.h>
#include <sti/utils/EvaluationBarrier.h>

#include "PersistenceTarget.h"

#include <vector>
#include <string>
#include <set>
#include <memory>
#include <functional>


namespace STI
{
namespace Device
{

class DeviceMessageDispatcher;
class TaskUpdateMessage;

class LocalTaskManager : public TaskManager,
						 public PersistenceTarget,
                         public STI::Utils::TaskSchedulerListener
{
public:

	LocalTaskManager(const STI::Device::DeviceID& localID = STI::Device::DeviceID(),
        const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher = nullptr);
    ~LocalTaskManager();

    void getTaskIDs(std::set<std::string>& ids) const;

    STI::Utils::TaskStatus getTaskStatus(const std::string& taskID) const;
    void setStatus(const std::string& taskID, const STI::Utils::TaskStatus& newStatus);

    bool getTask(const std::string& id, std::shared_ptr<STI::Utils::Task>& task) const;
    void getTasks(std::vector<std::shared_ptr<STI::Utils::Task>>& tasks) const;

	void addTask(const std::shared_ptr<STI::Utils::Task>& task);
	void removeTask(const std::string& taskID);
	void clear();

	void activateTask(const std::string& taskID);
	void deactivateTask(const std::string& taskID);

    void runTask(const std::string& taskID);

private:

    void handleEvent(const STI::Utils::TaskSchedulerEvent& evt);
    void sendTaskUpdate(const std::shared_ptr<STI::Device::TaskUpdateMessage>& message);
    
    STI::Device::DeviceID localID;
    std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;

    STI::Utils::EvaluationBarrier barrier;
    STI::Utils::TaskScheduler taskScheduler;

    //PersistenceTarget
    std::string getFilename();
    void setPersistenceCallback(const std::function<void(void)>& refresher);
    bool save(const std::string& filename);
    void load(const std::string& filename);

    std::function<void(void)> persistenceRefresher;

    //Persistence
    struct StoredTask
    {
        std::string taskID;
        STI::Utils::TaskStatus status;

        template<class Archive>
	    void serialize(Archive& archive);
    };

    struct StoredTasks
    {
        std::vector<StoredTask> tasks;

        template<class Archive>
        void serialize(Archive& archive);
    };

};


} //Device
} //STI

#endif
