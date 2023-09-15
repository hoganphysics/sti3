
#ifndef STI_DEVICE_JTASKMANAGER_H
#define STI_DEVICE_JTASKMANAGER_H

#include <sti/device/TaskManager.h>

#include <memory>


namespace STI
{
namespace Device
{

class TaskManager;


//Java TaskManager wrapper
class JTaskManager
{
public:
	
	JTaskManager(const std::shared_ptr<STI::Device::TaskManager>& manager);
	~JTaskManager();


    std::set<std::string> getTaskIDs() const;
    
    STI::Utils::TaskStatus getTaskStatus(const std::string& taskID) const;
    void setStatus(const std::string& taskID, const STI::Utils::TaskStatus& newStatus);

    std::shared_ptr<STI::Utils::Task> getTask(const std::string& taskID) const;
    std::vector<std::shared_ptr<STI::Utils::Task>> getTasks() const;

	void removeTask(const std::string& taskID);
	void clear();

	void activateTask(const std::string& taskID);
	void deactivateTask(const std::string& taskID);

    void runTask(const std::string& taskID);

private:

    std::shared_ptr<STI::Device::TaskManager> taskManager;

};

} //Device
} //STI

#endif
