#ifndef STI_ENGINE_EVENTENGINESCHEDULER_H
#define STI_ENGINE_EVENTENGINESCHEDULER_H

#include <sti/device/DeviceID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/device/DeviceTrace.h>
#include <sti/fwd/RawEvent_fwd.h>
#include <sti/engine/EventEngineJobList.h>

#include <memory>
#include <set>


namespace STI
{
namespace Engine
{

class EventEngineJob;
class EventEngineDependencyParser;
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
class SequenceID;
class Sequence;
class SequenceEntryID;


class EventEngineScheduler
{
public:

    virtual ~EventEngineScheduler() {}

    virtual ParseID parse(const std::shared_ptr<Shot>& shot) = 0;
    virtual ShotID play(const ParseID& parseID, const EngineJobSourceID& source) = 0;

    virtual SequenceID addSequence(const std::shared_ptr<Sequence>& sequence, const EngineJobSourceID& source) = 0;
    virtual ParseID parse(const std::shared_ptr<Shot>& shot, const SequenceEntryID& sequenceEntryID) = 0;
    // virtual ShotID play(const ParseID& parseID, const EngineJobSourceID& source, const SequenceEntryID& sequenceEntryID) = 0;

    virtual EngineJobStatus getStatus(const ParseID& pid) = 0;
    virtual EngineJobStatus getStatus(const ShotID& sid) = 0;

    virtual bool getDependencyParser(std::shared_ptr<EventEngineDependencyParser>& dependencyParser) = 0;

    virtual bool getJob(const EngineJobID& id, std::shared_ptr<EventEngineJob>& job) const = 0;    
    virtual void addJob(const std::shared_ptr<EventEngineJob>& newJob) = 0;
    virtual void cancelJob(const EngineJobID& jobID) = 0;

    virtual void cancelAll() = 0;

    // struct EventEngineJobFilter
    // {
    //     EventEngineJobList jobListType;
    //     int startIndex;
    //     int endIndex;
    // };

    virtual std::set<EngineJobID> getJobIDs(const EventEngineJobList& jobListType) const = 0;
    virtual std::vector<std::shared_ptr<EventEngineJob>> getJobs(const EventEngineJobList& jobListType) const = 0;

    virtual std::shared_ptr<Shot> createShot(const ShotConfig& shotConfig, const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup) = 0;

   	virtual void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) = 0;

    virtual bool getParseResult(const ParseID& parseID, std::shared_ptr<ParseResult>& parseResult) const = 0;

};


} //Engine
} //STI

#endif
