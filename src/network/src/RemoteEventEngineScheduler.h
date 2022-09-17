#ifndef STI_ENGINE_REMOTEEVENTENGINESCHEDULER_H
#define STI_ENGINE_REMOTEEVENTENGINESCHEDULER_H

#include <sti/engine/EventEngineScheduler.h>
#include <sti/device/DeviceMessage.h>
#include "deviceNet.h"

#include "TReferenceHolder.h"

#include <memory>
#include <mutex>
#include <set>


namespace STI
{
namespace Network
{

class RemoteEventEngineScheduler : public STI::Engine::EventEngineScheduler,
                                   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TEventEngineScheduler>	//mixin
{
public:

    RemoteEventEngineScheduler(::STI::TNetwork::TEventEngineScheduler_ptr scheduler);
    ~RemoteEventEngineScheduler();

    STI::Engine::ParseID parse(const std::shared_ptr<STI::Engine::Shot>& shot);
    STI::Engine::ShotID play(const STI::Engine::ParseID& parseID, const STI::Engine::EngineJobSourceID& source);

    STI::Engine::EngineJobStatus getStatus(const STI::Engine::ParseID& pid);
    STI::Engine::EngineJobStatus getStatus(const STI::Engine::ShotID& sid);

    void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, STI::Engine::EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<STI::Engine::EngineParsingMessage>& messages, 
                                const STI::Device::DeviceTrace& trace);

    void addDeviceEventTargets(STI::Engine::EventEngineDependencyTree& tree, 
                                std::vector<STI::Engine::EngineParsingMessage>& messages, const STI::Device::DeviceTrace& trace);
    
    bool getJob(const STI::Engine::EngineJobID& id, std::shared_ptr<STI::Engine::EventEngineJob>& job) const;
    void addJob(const std::shared_ptr<STI::Engine::EventEngineJob>& newJob);
    void cancelJob(const STI::Engine::EngineJobID& jobID);

    void cancelAll();

    std::set<STI::Engine::EngineJobID> getJobIDs(const STI::Engine::EventEngineJobList& jobListType) const;
    std::vector<std::shared_ptr<STI::Engine::EventEngineJob>> getJobs(const STI::Engine::EventEngineJobList& jobListType) const;

    // void getQueuedJobs(std::set<STI::Engine::EngineJobID>& jobIDs) const;
    // void getRunningJobs(std::set<STI::Engine::EngineJobID>& jobIDs) const;
    // void getCompletedJobs(std::set<STI::Engine::EngineJobID>& jobIDs) const;

    std::shared_ptr<STI::Engine::Shot> createShot(const STI::Engine::ShotConfig& shotConfig, const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup);

	void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) {}
	
    bool getParseResult(const STI::Engine::ParseID& parseID, std::shared_ptr<STI::Engine::ParseResult>& parseResult) const;

    bool ping() const;

private:

    mutable std::mutex schedulerMutex;

};


} //Network
} //STI

#endif
