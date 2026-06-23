#ifndef STI_ENGINE_LOCALEVENTENGINEJOB_H
#define STI_ENGINE_LOCALEVENTENGINEJOB_H

#include <sti/engine/EventEngineJob.h>

#include <sti/device/DeviceID.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineJobStatus.h>

#include <sti/engine/Shot.h>

#include <set>
#include <map>
#include <memory>
#include <mutex>
#include <vector>


namespace STI
{
namespace Engine
{

class EventEngineDependencyTree;
class EventEngine;
class EngineParsingMessage;
enum class ParsingMessageType;
class EnginePlayingMessage;
enum class PlayingMessageType;


class LocalEventEngineJob : public EventEngineJob
{
public:

    //Parse jobs
    LocalEventEngineJob(const ParseID& parseID, 
                        const std::shared_ptr<Shot>& shot,
                        const STI::Device::DeviceID& owner);

    //Play jobs
    LocalEventEngineJob(const EngineJobID& id, 
                        const std::shared_ptr<Shot>& shot,
                        const STI::Device::DeviceID& owner);

    EngineJobID getJobID() const;
    STI::Device::DeviceID getJobOwner() const;
    EngineJobStatus getStatus() const;

    void setStatus(const EngineJobStatus& jobStatus);

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

    void addPlayMessages(const std::vector<EnginePlayingMessage>& messages);
    EnginePlayingMessage& addPlayMessage(const EnginePlayingMessage& message);
    EnginePlayingMessage& addPlayMessage(const PlayingMessageType& type, unsigned id, const std::string& name);

    const std::vector<EnginePlayingMessage>& getPlayMessages() const { return playingMessages; }

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
    std::vector<EnginePlayingMessage> playingMessages;

	mutable std::mutex jobMutex;
};


} //Engine
} //STI

#endif
