#ifndef STI_TNETWORK_TTASKMANAGER_I_H
#define STI_TNETWORK_TTASKMANAGER_I_H

#include <sti/device/TaskManager.h>
#include <sti/device/Device.h>
#include "generated/tasks.h"

#include <memory>


namespace STI
{
namespace TNetwork
{


class TTaskManager_i : public POA_STI::TNetwork::TTaskManager
{
public:

    TTaskManager_i(const std::shared_ptr<STI::Device::Device>& device);
	~TTaskManager_i();
    
    void getTaskIDs(::STI::TNetwork::TStringSeq_out ids);
    TTaskStatus getTaskStatus(const char* taskID);
    void setStatus(const char* taskID, ::STI::TNetwork::TTaskStatus newStatus);
    ::CORBA::Boolean getTask(const char* taskID, ::STI::TNetwork::TTask_out task);
    void getTasks(::STI::TNetwork::TTaskSeq_out tasks);
    void removeTask(const char* taskID);
    void clear();
    void activateTask(const char* taskID);
    void deactivateTask(const char* taskID);
    void runTask(const char* taskID);
    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Device::TaskManager> taskManager;

};


} //TNetwork
} //STI

#endif

