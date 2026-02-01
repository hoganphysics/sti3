#ifndef STI_ENGINE_REMOTEEVENTENGINESCHEDULER_H
#define STI_ENGINE_REMOTEEVENTENGINESCHEDULER_H

#include <sti/engine/EventEngineScheduler.h>
#include <sti/device/DeviceMessage.h>
#include "generated/deviceNet.h"

#include "TReferenceHolder.h"

#include <memory>
#include <mutex>
#include <set>


namespace STI
{
namespace Network
{

class RemoteEventEngineDependencyParser;


class RemoteEventEngineScheduler : public STI::Engine::EventEngineScheduler,
                                   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TEventEngineScheduler>	//mixin
{
public:

    RemoteEventEngineScheduler(::STI::TNetwork::TEventEngineScheduler_var scheduler);
    ~RemoteEventEngineScheduler();

    STI::Engine::ParseJobStatus parse(const std::shared_ptr<STI::Engine::Shot>& shot);
    STI::Engine::ParseJobStatus parse(const std::shared_ptr<STI::Engine::Shot>& shot, const STI::Engine::SequenceEntryID& sequenceEntryID);
    STI::Engine::ParseJobStatus parse(const std::shared_ptr<STI::Engine::Shot>& shot, const STI::Engine::SequenceID& sequenceID);

    STI::Engine::PlayJobStatus play(const STI::Engine::ParseID& parseID, const STI::Engine::EngineJobSourceID& source);

    STI::Engine::AddSequenceStatus addSequence(const std::shared_ptr<STI::Engine::Sequence>& sequence, const STI::Engine::EngineJobSourceID& source);

    void closeSequence(const STI::Engine::SequenceID& seqid);
    void cancelSequence(const STI::Engine::SequenceID& seqid);

    STI::Engine::EngineJobStatus getStatus(const STI::Engine::ParseID& pid);
    STI::Engine::EngineJobStatus getStatus(const STI::Engine::ShotID& sid);
    STI::Engine::EngineJobStatus getStatus(const STI::Engine::SequenceID& seqID);

    bool getDependencyParser(std::shared_ptr<STI::Engine::EventEngineDependencyParser>& dependencyParser);

    bool getJob(const STI::Engine::EngineJobID& id, std::shared_ptr<STI::Engine::EventEngineJob>& job) const;
    void addJob(const std::shared_ptr<STI::Engine::EventEngineJob>& newJob);
    void cancelJob(const STI::Engine::EngineJobID& jobID);

    void cancelAll();

    std::set<STI::Engine::EngineJobID> getJobIDs(const STI::Engine::EventEngineJobList& jobListType) const;
    std::vector<std::shared_ptr<STI::Engine::EventEngineJob>> getJobs(const STI::Engine::EventEngineJobList& jobListType) const;

    std::shared_ptr<STI::Engine::Shot> createShot(const STI::Engine::ShotConfig& shotConfig, const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup);

	void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) {}
	
    void getEngineIDs(std::set<STI::Engine::EngineID>& engineIDs) const;
    STI::Engine::EngineState getEngineState(const STI::Engine::EngineID& engineID) const;
    void getEngineStates(std::map<STI::Engine::EngineID, STI::Engine::EngineState>& engineStates) const;

    void clearEngine(const STI::Engine::EngineID& engineID);
    void stopEngine(const STI::Engine::EngineID& engineID);

    bool getParseResult(const STI::Engine::ParseID& parseID, std::shared_ptr<STI::Engine::ParseResult>& parseResult) const;
    bool getShotResult(const STI::Engine::ShotID& shotID, std::shared_ptr<STI::Engine::ShotResult>& shotResult) const;

    bool ping() const;

private:

    mutable std::mutex schedulerMutex;

    std::shared_ptr<RemoteEventEngineDependencyParser> remoteDependencyParser;
};


} //Network
} //STI

#endif
