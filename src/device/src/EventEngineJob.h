#ifndef STI_ENGINE_EVENTENGINEJOB_H
#define STI_ENGINE_EVENTENGINEJOB_H


#include "EngineJobID.h"
#include "DeviceID.h"
#include "ParsedShot.h"
#include "EngineID.h"

#include <set>
#include <map>
#include <memory>
#include <mutex>

namespace STI
{
namespace Engine
{

class EventEngineDependencyTree;
class EventEngine;


class EventEngineJob
{
public:

    enum class EngineJobStatus { New, Running, Completed, Cancelled };

    //Parse jobs
    EventEngineJob(const ParseID& parseID, 
                   const std::shared_ptr<ParsedShot>& shot,
                   const std::shared_ptr<EventEngineDependencyTree>& tree, 
                   const STI::Device::DeviceID& owner, 
                   const std::set<STI::Device::DeviceID>& missingTargets);

    //Play jobs
    EventEngineJob(const EngineJobID& id, 
                   const STI::Device::DeviceID& owner);

    EngineJobID getJobID() const;

    EngineJobStatus getStatus() const;
    void markRunning(const EngineID& id);
    void markComplete();
    void markCancelled();

    const EngineID& getEngineID() const { return engineID; }
    std::shared_ptr<EventEngine> getEngine() const { return engine; }
    void setEventEngine(const std::shared_ptr<EventEngine>& eventEngine) { engine = eventEngine; }
    
    std::shared_ptr<ParsedShot> parsedShot;
    std::shared_ptr<EventEngineDependencyTree> dependencies;
    STI::Device::DeviceID jobOwner;
    std::set<STI::Device::DeviceID> missingTargetIDs;
    
private:

    EngineJobID jobID;
    EngineID engineID;
    std::shared_ptr<EventEngine> engine;

    EngineJobStatus status;

	mutable std::mutex jobMutex;
};


} //Engine
} //STI

#endif
