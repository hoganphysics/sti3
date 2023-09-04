#ifndef STI_DEVICE_LOCALTASKMANAGER_H
#define STI_DEVICE_LOCALTASKMANAGER_H

#include <sti/device/TaskManager.h>
#include <sti/utils/TaskScheduler.h>

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


class LocalTaskManager : public TaskManager,
						 public PersistenceTarget
{
public:

	LocalTaskManager();
    ~LocalTaskManager();

    void getTaskIDs(std::set<std::string>& ids);
    STI::Utils::TaskStatus getTaskStatus(const std::string& taskID);

    bool getTask(const std::string& id, std::shared_ptr<STI::Utils::Task>& task);
    void getTasks(std::vector<std::shared_ptr<STI::Utils::Task>>& tasks);

	void addTask(const std::shared_ptr<STI::Utils::Task>& task);
	void removeTask(const std::string& taskID);
	void clear();

	void activateTask(const std::string& taskID);
	void deactivateTask(const std::string& taskID);

    void runTask(const std::string& taskID);

private:

    STI::Utils::TaskScheduler taskScheduler;

    //PersistenceTarget
    std::string getFilename();
    // void setLoadFilename(const std::string& filename);
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
