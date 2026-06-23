#ifndef STI_DEVICE_REMOTETASK_H
#define STI_DEVICE_REMOTETASK_H

#include <sti/utils/Task.h>
#include <sti/utils/MetaData.h>

#include <string>
#include <optional>
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
    RemoteTask(const std::string& id, const STI::Utils::MixedValue& metaData,
        const std::optional<STI::Utils::TimeStamp>& snapshotLastRunTime);
    ~RemoteTask();

    void attachManager(RemoteTaskManager* manager);
	
    bool isActive() const;
	STI::Utils::TaskStatus getStatus() const;
    bool hasLastRunTime() const override;
    std::optional<STI::Utils::TimeStamp> getLastRunTime() const override;

private:

	double secondsToNextRun() const;
	void run();
	void skipTask() {}
	bool repeat() { return false; }

    friend RemoteTaskManager;
    RemoteTaskManager* remoteManager;
    std::optional<STI::Utils::TimeStamp> snapshotLastRunTime;
};


} //Network
} //STI

#endif
