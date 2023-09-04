#ifndef STI_DEVICE_REMOTETASK_H
#define STI_DEVICE_REMOTETASK_H

#include <sti/utils/Task.h>
#include <sti/utils/MetaData.h>

#include <string>
#include <vector>


namespace STI
{
namespace Network
{

class RemoteTaskManager;

class RemoteTask : public STI::Utils::Task
{
public:

    RemoteTask(const std::string& id, const STI::Utils::MixedValue& metaData);
    ~RemoteTask();

    void attachManager(RemoteTaskManager* manager);
	
    bool isActive() const;
	STI::Utils::TaskStatus getStatus() const;
	void setStatus(const STI::Utils::TaskStatus& newStatus);

private:

	double secondsToNextRun() const { return 0; }
	void run();
	void skipTask() {}
	bool repeat() { return false; }

    friend RemoteTaskManager;
    RemoteTaskManager* remoteManager;
};


} //Network
} //STI

#endif
