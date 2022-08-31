#ifndef STI_ENGINE_EVENTENGINESCHEDULER_H
#define STI_ENGINE_EVENTENGINESCHEDULER_H

#include <sti/device/DeviceID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/device/DeviceTrace.h>
#include <sti/fwd/RawEvent_fwd.h>

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
class ResultsCollector;
class ResultTicket;
class ParsedDependencyTree;
class EngineJobSourceID;
class ShotConfig;
class ParseResult;


class EventEngineScheduler
{
public:

    virtual ~EventEngineScheduler() {}

    //possibly unneeded?
    //virtual void parse(const ParseID& parseID, const std::shared_ptr<Shot>& shot) = 0;
    //virtual void play(const ShotID& shotID) = 0;

    virtual ParseID parse(const std::shared_ptr<Shot>& shot) = 0;
    virtual ShotID play(const ParseID& parseID, const EngineJobSourceID& source) = 0;

    virtual EngineJobStatus getStatus(const ParseID& pid) = 0;
    virtual EngineJobStatus getStatus(const ShotID& sid) = 0;

    virtual void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages, 
                                const STI::Device::DeviceTrace& trace) = 0;
    
    virtual void addDeviceEventTargets(EventEngineDependencyTree& tree, 
                                        std::vector<EngineParsingMessage>& messages, 
                                        const STI::Device::DeviceTrace& trace) = 0;
    
    virtual void addJob(const std::shared_ptr<EventEngineJob>& newJob) = 0;
    virtual void cancelJob(const EngineJobID& jobID) = 0;

    virtual void cancelAll() = 0;

    virtual void getQueuedJobs(std::set<EngineJobID>& jobIDs) const = 0;
    virtual void getRunningJobs(std::set<EngineJobID>& jobIDs) const = 0;
    virtual void getCompletedJobs(std::set<EngineJobID>& jobIDs) const = 0;


    // virtual std::shared_ptr<Shot> createShot(const ShotConfig& shotConfig) = 0;
    virtual std::shared_ptr<Shot> createShot(const ShotConfig& shotConfig, const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup) = 0;

   	virtual void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) = 0;

    // virtual bool getParsedEvents(const ParseID& parseID, DeviceEventMap& events) const = 0;
    // virtual bool getParsingMessages(const ParseID& parseID, std::vector<EngineParsingMessage>& messages) const = 0;
    // virtual bool getParsedTree(const ParseID& parseID, std::shared_ptr<ParsedDependencyTree>& tree) const = 0;
    
    virtual bool getParseResult(const ParseID& parseID, std::shared_ptr<ParseResult>& parseResult) const = 0;

    // virtual bool transferResults(const std::shared_ptr<ResultsCollector>& resultsCollector) = 0;
    // virtual bool getResults(const ShotID& shotID, std::shared_ptr<ResultTicket>& results) = 0;
};


} //Engine
} //STI

#endif
