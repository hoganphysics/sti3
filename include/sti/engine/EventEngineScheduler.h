#ifndef STI_ENGINE_EVENTENGINESCHEDULER_H
#define STI_ENGINE_EVENTENGINESCHEDULER_H

#include <sti/device/DeviceID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/device/DeviceTrace.h>
#include <sti/fwd/RawEvent_fwd.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineState.h>
#include <sti/engine/EventEngineJobList.h>

#include <memory>
#include <set>
#include <map>
#include <vector>

namespace STI
{
namespace Engine
{

class EventEngineJob;
class EventEngineDependencyParser;
class EventEngineDependencyTree;
class PostProcessRequest;
class Shot;
class ParseID;
class EngineJobID;
class EventEngineFactory;
class ShotID;
class ShotResult;
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
class ParseJobStatus;
class PlayJobStatus;
class AddSequenceStatus;


class EventEngineScheduler
{
public:

    virtual ~EventEngineScheduler() {}

    virtual ParseJobStatus parse(const std::shared_ptr<Shot>& shot) = 0;
    virtual ParseJobStatus parse(const std::shared_ptr<Shot>& shot, const SequenceEntryID& sequenceEntryID) = 0;
    virtual ParseJobStatus parse(const std::shared_ptr<Shot>& shot, const SequenceID& sequenceID) = 0;

    virtual PlayJobStatus play(const ParseID& parseID, const EngineJobSourceID& source) = 0;

    virtual AddSequenceStatus addSequence(const std::shared_ptr<Sequence>& sequence, const EngineJobSourceID& source) = 0;
    virtual void closeSequence(const SequenceID& seqid) = 0;
    virtual void cancelSequence(const SequenceID& seqid) = 0;

    virtual EngineJobStatus getStatus(const ParseID& pid) = 0;
    virtual EngineJobStatus getStatus(const ShotID& sid) = 0;
    virtual EngineJobStatus getStatus(const SequenceID& seqID) = 0;

    virtual bool getDependencyParser(std::shared_ptr<EventEngineDependencyParser>& dependencyParser) = 0;

    virtual bool getJob(const EngineJobID& id, std::shared_ptr<EventEngineJob>& job) const = 0;
    virtual void addJob(const std::shared_ptr<EventEngineJob>& newJob) = 0;
    virtual void cancelJob(const EngineJobID& jobID) = 0;
    virtual void cancelAll() = 0;

    //Tree-routed post-processing dispatch: deliver/forward the request sublist toward
    //each target along the ownership chain described by `tree`. Thin: each delivered
    //requestPostProcessing only enqueues and returns (the analysis runs async on the
    //target's worker thread). See docs/notes/postProcess-hierarchical-refactor.md.
    virtual void distributePostProcessing(const std::vector<PostProcessRequest>& requests,
                                          const std::shared_ptr<EventEngineDependencyTree>& tree,
                                          const ShotID& shotID, const STI::Device::DeviceID& jobOwnerID) = 0;

    virtual std::set<EngineJobID> getJobIDs(const EventEngineJobList& jobListType) const = 0;
    virtual std::vector<std::shared_ptr<EventEngineJob>> getJobs(const EventEngineJobList& jobListType) const = 0;

    virtual std::shared_ptr<Shot> createShot(const ShotConfig& shotConfig, const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup) = 0;

   	virtual void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) = 0;

    virtual void getEngineIDs(std::set<EngineID>& engineIDs) const = 0;
    virtual EngineState getEngineState(const EngineID& engineID) const = 0;
    virtual void getEngineStates(std::map<EngineID, EngineState>& engineStates) const = 0;

    virtual void clearEngine(const EngineID& engineID) = 0;
    virtual void stopEngine(const EngineID& engineID) = 0;

    virtual bool getParseResult(const ParseID& parseID, std::shared_ptr<ParseResult>& parseResult) const = 0;
    virtual bool getShotResult(const ShotID& shotID, std::shared_ptr<ShotResult>& shotResult) const = 0;
    virtual bool getLastParseResult(const EngineID& engineID, std::shared_ptr<ParseResult>& parseResult) const = 0;
    virtual bool getLastShotResult(const EngineID& engineID, std::shared_ptr<ShotResult>& shotResult) const = 0;

};


} //Engine
} //STI

#endif
