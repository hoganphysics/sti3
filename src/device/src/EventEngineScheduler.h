#ifndef STI_ENGINE_EVENTENGINESCHEDULER_H
#define STI_ENGINE_EVENTENGINESCHEDULER_H

#include "DeviceID.h"
#include "DeviceTrace.h"
#include "fwd/RawEvent_fwd.h"

#include <memory>
#include <set>


namespace STI
{
namespace Engine
{

class EventEngineJob;
class EventEngineDependencyTree;
class Shot;
class ParseID;
class EngineJobID;
class EventEngineFactory;
class ShotID;
class EngineParsingMessage;


class EventEngineScheduler
{
public:

    virtual ~EventEngineScheduler() {}

    virtual void parse(const ParseID& parseID, const std::shared_ptr<Shot>& shot) = 0;
    virtual void play(const ShotID& shotID) = 0;

    virtual void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages, 
                                const STI::Device::DeviceTrace& trace) = 0;
    
    virtual void addDeviceEventTargets(EventEngineDependencyTree& tree, 
                                        std::vector<EngineParsingMessage>& messages, 
                                        const STI::Device::DeviceTrace& trace) = 0;
    
    virtual void addJob(const std::shared_ptr<EventEngineJob>& newJob) = 0;
    virtual void cancelJob(const EngineJobID& jobID) = 0;

    virtual void cancelAll() = 0;

    virtual std::shared_ptr<Shot> createShot(const std::shared_ptr<RawEventVector>& events) = 0;

   	virtual void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) = 0;

    virtual bool getParsedEvents(const ParseID& parseID, DeviceEventMap& events) const = 0;
    virtual bool getParsingMessages(const ParseID& parseID, std::vector<EngineParsingMessage>& messages) const = 0;
    virtual bool getParsedTree(const ParseID& parseID, std::shared_ptr<EventEngineDependencyTree>& tree) const = 0;
};



} //Engine
} //STI

#endif
