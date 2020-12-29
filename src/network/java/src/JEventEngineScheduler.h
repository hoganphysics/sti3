#ifndef STI_DEVICE_JEVENTENGINESCHEDULER_H
#define STI_DEVICE_JEVENTENGINESCHEDULER_H

#include "EventEngineScheduler.h"
#include "ParsedShot.h"
#include "ParseID.h"

#include <memory>
#include <set>

namespace STI
{
namespace Device
{

//Java JEventEngineScheduler wrapper
class JEventEngineScheduler //: public STI::Engine::EventEngineScheduler
{
public:
	
	JEventEngineScheduler(const std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	~JEventEngineScheduler();

    //EventEngineScheduler

    STI::Device::DeviceID id;

    void parse(const STI::Engine::ParseID& parseID, const std::shared_ptr<STI::Engine::ParsedShot>& shot);
    void play(const STI::Engine::ShotID& shotID);
    void cancelJob(const STI::Engine::EngineJobID& jobID);

private:

    //EventEngineScheduler
    void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, STI::Engine::EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, const STI::Device::DeviceTrace& trace);
    
    void addDeviceEventTargets(STI::Engine::EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace);
    
    void addJob(const std::shared_ptr<STI::Engine::EventEngineJob>& newJob);


    std::shared_ptr<STI::Engine::EventEngineJob> createJob(const STI::Engine::ParseID& parseID, 
                                                      const std::shared_ptr<STI::Engine::ParsedShot>& shot,
                                                      const std::shared_ptr<STI::Engine::EventEngineDependencyTree>& tree, 
                                                      const STI::Device::DeviceID& owner, 
                                                      const std::set<STI::Device::DeviceID>& missingTargets);


    std::shared_ptr<STI::Engine::EventEngineScheduler> localScheduler;

};

} //Engine
} //STI

#endif
