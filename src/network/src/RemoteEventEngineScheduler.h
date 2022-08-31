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
    
    void addJob(const std::shared_ptr<STI::Engine::EventEngineJob>& newJob);
    void cancelJob(const STI::Engine::EngineJobID& jobID);

    void cancelAll();

    void getQueuedJobs(std::set<STI::Engine::EngineJobID>& jobIDs) const;
    void getRunningJobs(std::set<STI::Engine::EngineJobID>& jobIDs) const;
    void getCompletedJobs(std::set<STI::Engine::EngineJobID>& jobIDs) const;

    // std::shared_ptr<STI::Engine::EventEngineJob> createJob(const STI::Engine::ParseID& parseID, 
    //                                           const std::shared_ptr<STI::Engine::Shot>& shot,
    //                                           const std::shared_ptr<STI::Engine::EventEngineDependencyTree>& tree, 
    //                                           const STI::Device::DeviceID& owner, 
    //                                           const std::set<STI::Device::DeviceID>& missingTargets);

    std::shared_ptr<STI::Engine::Shot> createShot(const STI::Engine::ShotConfig& shotConfig, const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup);

	void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) {}

    // bool getParsedEvents(const STI::Engine::ParseID& parseID, STI::Engine::DeviceEventMap& events) const;
    // bool getParsingMessages(const STI::Engine::ParseID& parseID, std::vector<STI::Engine::EngineParsingMessage>& messages) const;
    // bool getParsedTree(const STI::Engine::ParseID& parseID, std::shared_ptr<STI::Engine::ParsedDependencyTree>& tree) const;
	
    bool getParseResult(const STI::Engine::ParseID& parseID, std::shared_ptr<STI::Engine::ParseResult>& parseResult) const;


    bool ping() const;

private:

	//::STI::TNetwork::TEventEngineScheduler_var tEventEngineScheduler;		//remote reference

    mutable std::mutex schedulerMutex;

};



} //Network
} //STI

#endif
