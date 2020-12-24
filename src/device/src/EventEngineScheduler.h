#ifndef STI_ENGINE_EVENTENGINESCHEDULER_H
#define STI_ENGINE_EVENTENGINESCHEDULER_H

#include "DeviceID.h"
#include "DeviceTrace.h"

#include <memory>
#include <set>

namespace STI
{
namespace Engine
{

class EventEngineJob;
class EventEngineDependencyTree;
class ParsedShot;
class ParseID;
class EngineJobID;
class EventEngineFactory;

class EventEngineScheduler
{
public:

    virtual ~EventEngineScheduler() {}

    virtual void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, const STI::Device::DeviceTrace& trace) = 0;
    
    virtual void addDeviceEventTargets(EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace) = 0;
    
    virtual void addJob(const std::shared_ptr<EventEngineJob>& newJob) = 0;
    virtual void cancelJob(const EngineJobID& jobID) = 0;

    virtual std::shared_ptr<EventEngineJob> createJob(const ParseID& parseID, 
                                                      const std::shared_ptr<ParsedShot>& shot,
                                                      const std::shared_ptr<EventEngineDependencyTree>& tree, 
                                                      const STI::Device::DeviceID& owner, 
                                                      const std::set<STI::Device::DeviceID>& missingTargets) = 0;

   	virtual void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) = 0;

};



} //Engine
} //STI

#endif
