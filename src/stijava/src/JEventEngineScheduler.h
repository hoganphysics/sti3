#ifndef STI_DEVICE_JEVENTENGINESCHEDULER_H
#define STI_DEVICE_JEVENTENGINESCHEDULER_H

#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/Shot.h>
#include <sti/engine/ParseID.h>

#include <memory>
#include <set>

namespace STI
{
namespace Engine
{

class JShot;


//Java JEventEngineScheduler wrapper
class JEventEngineScheduler //: public STI::Engine::EventEngineScheduler
{
public:
	
	JEventEngineScheduler(const std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	~JEventEngineScheduler();

    //EventEngineScheduler

    // STI::Device::DeviceID id;

    STI::Engine::ParseID parse(const std::shared_ptr<STI::Engine::JShot>& shot);
    STI::Engine::ShotID play(const ParseID& parseID, const EngineJobSourceID& source);
    void cancelJob(const STI::Engine::EngineJobID& jobID);

    void cancelAll();

    std::set<EngineJobID> getQueuedJobs() const;
    std::set<EngineJobID> getRunningJobs() const;
    std::set<EngineJobID> getCompletedJobs() const;

private:

    //EventEngineScheduler
    // void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, STI::Engine::EventEngineDependencyTree& tree, 
    //                             std::set<STI::Device::DeviceID>& missingTargets, const STI::Device::DeviceTrace& trace);
    
    // void addDeviceEventTargets(STI::Engine::EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace);
    
    // void addJob(const std::shared_ptr<STI::Engine::EventEngineJob>& newJob);


    // std::shared_ptr<STI::Engine::EventEngineJob> createJob(const STI::Engine::ParseID& parseID, 
    //                                                   const std::shared_ptr<STI::Engine::Shot>& shot,
    //                                                   const std::shared_ptr<STI::Engine::EventEngineDependencyTree>& tree, 
    //                                                   const STI::Device::DeviceID& owner, 
    //                                                   const std::set<STI::Device::DeviceID>& missingTargets);


    std::shared_ptr<STI::Engine::EventEngineScheduler> localScheduler;

};

} //Engine
} //STI

#endif
