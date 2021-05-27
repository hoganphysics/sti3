#ifndef STI_ENGINE_REMOTEEVENTENGINESCHEDULER_H
#define STI_ENGINE_REMOTEEVENTENGINESCHEDULER_H

#include "EventEngineScheduler.h"
#include "DeviceMessage.h"
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

    void parse(const STI::Engine::ParseID& parseID, const std::shared_ptr<STI::Engine::Shot>& shot);
    void play(const STI::Engine::ShotID& shotID);

    
    void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, STI::Engine::EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<STI::Engine::EngineParsingMessage>& messages, 
                                const STI::Device::DeviceTrace& trace);

    void addDeviceEventTargets(STI::Engine::EventEngineDependencyTree& tree, 
                                std::vector<STI::Engine::EngineParsingMessage>& messages, const STI::Device::DeviceTrace& trace);
    
    void addJob(const std::shared_ptr<STI::Engine::EventEngineJob>& newJob);
    void cancelJob(const STI::Engine::EngineJobID& jobID);

    void cancelAll();

    // std::shared_ptr<STI::Engine::EventEngineJob> createJob(const STI::Engine::ParseID& parseID, 
    //                                           const std::shared_ptr<STI::Engine::Shot>& shot,
    //                                           const std::shared_ptr<STI::Engine::EventEngineDependencyTree>& tree, 
    //                                           const STI::Device::DeviceID& owner, 
    //                                           const std::set<STI::Device::DeviceID>& missingTargets);

    std::shared_ptr<STI::Engine::Shot> createShot(const std::shared_ptr<STI::Engine::RawEventVector>& events);

	void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) {}

    bool getParsedEvents(const STI::Engine::ParseID& parseID, STI::Engine::DeviceEventMap& events) const;
    bool getParsingMessages(const STI::Engine::ParseID& parseID, std::vector<STI::Engine::EngineParsingMessage>& messages) const;
    bool getParsedTree(const STI::Engine::ParseID& parseID, std::shared_ptr<STI::Engine::EventEngineDependencyTree>& tree) const;
	
    bool ping() const;

private:

	//::STI::TNetwork::TEventEngineScheduler_var tEventEngineScheduler;		//remote reference

    mutable std::mutex schedulerMutex;

};



} //Network
} //STI

#endif
