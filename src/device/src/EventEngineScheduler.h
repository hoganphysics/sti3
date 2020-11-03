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

class EventEngineScheduler
{
public:

    virtual ~EventEngineScheduler() {}

    virtual void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, const STI::Device::DeviceTrace& trace) = 0;
    
    virtual void addDeviceEventTargets(EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace) = 0;
    
    virtual void addJob(const std::shared_ptr<EventEngineJob>& newJob) = 0;

};



} //Engine
} //STI

#endif
