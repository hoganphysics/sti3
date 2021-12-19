#ifndef STI_ENGINE_LOCALEVENTENGINEJOB_H
#define STI_ENGINE_LOCALEVENTENGINEJOB_H

#include "EventEngineJob.h"

#include "EngineJobStatus.h"
#include "EngineJobID.h"
#include "DeviceID.h"
#include "Shot.h"
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
class EngineParsingMessage;
enum class ParsingMessageType;


class LocalEventEngineJob : public EventEngineJob
{
public:

    //Parse jobs
    LocalEventEngineJob(const ParseID& parseID, 
                        const std::shared_ptr<Shot>& shot,
                        const STI::Device::DeviceID& owner);

    //Play jobs
    LocalEventEngineJob(const EngineJobID& id, 
                        const STI::Device::DeviceID& owner);

    EngineJobID getJobID() const;
    STI::Device::DeviceID getJobOwner() const;
    EngineJobStatus getStatus() const;

    void markRunning(const EngineID& id);
    void markComplete();
    void markCancelled();
    void markArchived();

    void attachSubjob(const std::shared_ptr<EventEngineJob>& job);

    const EngineID& getEngineID() const { return engineID; }
    bool getEngine(std::shared_ptr<EventEngine>& eventEngine) const { eventEngine = engine; return (eventEngine != 0); }
    void setEventEngine(const std::shared_ptr<EventEngine>& eventEngine) { engine = eventEngine; }

    bool getShot(std::shared_ptr<Shot>& shot) const;
    bool getDependencies(std::shared_ptr<EventEngineDependencyTree>& tree) const;

    std::set<STI::Device::DeviceID> getMissingTargetIDs() const;
    
    void setDependencies(const std::shared_ptr<EventEngineDependencyTree>& tree);
    void setMissingTargets(const std::set<STI::Device::DeviceID>& missingTargets);

    void addMessages(const std::vector<EngineParsingMessage>& messages);
    EngineParsingMessage& addMessage(const EngineParsingMessage& message);
    EngineParsingMessage& addMessage(const ParsingMessageType& type, unsigned id, const std::string& name);

    const std::vector<EngineParsingMessage>& getParsingMessages() const { return parsingMessages; }

private:

    STI::Device::DeviceID jobOwner;    
    std::shared_ptr<Shot> shot_;
    std::shared_ptr<EventEngineDependencyTree> dependencies;
    std::set<STI::Device::DeviceID> missingTargetIDs;

    EngineJobID jobID;
    EngineID engineID;
    std::shared_ptr<EventEngine> engine;

    EngineJobStatus status;

    std::vector<std::shared_ptr<EventEngineJob>> attachedJobs;

    std::vector<EngineParsingMessage> parsingMessages;

	mutable std::mutex jobMutex;
};


} //Engine
} //STI

#endif
