#ifndef STI_ENGINE_REMOTEEVENTENGINESCHEDULER_H
#define STI_ENGINE_REMOTEEVENTENGINESCHEDULER_H

#include "EventEngineScheduler.h"

#include "deviceNet.h"

#include <memory>
#include <set>

namespace STI
{
namespace Network
{

class RemoteEventEngineScheduler : public STI::Engine::EventEngineScheduler
{
public:

    RemoteEventEngineScheduler(::STI::TNetwork::TEventEngineScheduler_ptr scheduler);
    ~RemoteEventEngineScheduler();

    void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, STI::Engine::EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, const STI::Device::DeviceTrace& trace);
    
    void addDeviceEventTargets(STI::Engine::EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace);
    
    void addJob(const std::shared_ptr<STI::Engine::EventEngineJob>& newJob);
    void cancelJob(const STI::Engine::EngineJobID& jobID);

    std::shared_ptr<STI::Engine::EventEngineJob> createJob(const STI::Engine::ParseID& parseID, 
                                              const std::shared_ptr<STI::Engine::ParsedShot>& shot,
                                              const std::shared_ptr<STI::Engine::EventEngineDependencyTree>& tree, 
                                              const STI::Device::DeviceID& owner, 
                                              const std::set<STI::Device::DeviceID>& missingTargets);

	void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) {}

private:

	::STI::TNetwork::TEventEngineScheduler_var tEventEngineScheduler;		//remote reference

};



} //Network
} //STI

#endif
