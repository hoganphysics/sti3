#ifndef STI_ENGINE_LOCALEVENTENGINESCHEDULER_H
#define STI_ENGINE_LOCALEVENTENGINESCHEDULER_H

#include <sti/LocalDevice.h>

#include <sti/device/Device.h>
#include <sti/device/DeviceTrace.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageListener.h>

#include <sti/engine/EngineID.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/ParseID.h>

#include <sti/utils/SynchronizedMap.h>

#include "EventEngineDependencyTree.h"
#include "LocalEventEngine.h"
#include "MessageGenerator.h"
#include <sti/engine/Shot.h>
#include "utils/OrderedBufferMap.h"
#include <sti/utils/VirtualFileServer.h>

#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <string>

/*
Implements prioritized parallel distributed scheduling:
- Prioritized: Each job has an absolute priority rank and jobs proceed in order.
- Parallel: Multiple jobs can be run in parallel, depending on overlap restrictions of the specific jobs.
- Distributed: Jobs are run synchronously across the network, without global network information (only local knowledge at each node).
- Scheduling: Jobs (play/parse) are added to a queue and run when required resources are available, based on priority.
*/

    // Rules:
    // 1) Each engine can only do one thing at a time.
    // 2) Only one engine can play at a time.
    // 3) An engine must be parsed with the correct ID before it can play that ID.
    //      3b) If an ID is not parsed, it cannot be played.
    // (with the exception of async engine... TBD)

    /*
    Wait until refresh. Wake with (event complete on any thread; add event; cancel event;). On wake:
    - If playing, wait.
    - If 
    
    */

namespace STI
{
namespace Engine
{

class EngineJobID;
class LocalEventEngineDependencyParser;
class EventEngineFactory;
class EventEngineJob;
class EventEngineManager;
class LocalEventEngine;
class ParseID;
class Shot;
class ShotResult;
class LocalEventEngineJob;
class SequenceJob;
class EngineConflictPolicy;


class LocalEventEngineScheduler : public EventEngineScheduler,
                                  public STI::Device::MessageGenerator
{
public:
    
    LocalEventEngineScheduler(STI::Device::LocalDevice* localDevice, 
                                const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory,
                                const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher,
                                const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager);
    ~LocalEventEngineScheduler();

    ParseJobStatus parse(const std::shared_ptr<Shot>& shot);        //local; add event to queue
    ParseJobStatus parse(const std::shared_ptr<Shot>& shot, const SequenceEntryID& sequenceEntryID);
    ParseJobStatus parse(const std::shared_ptr<Shot>& shot, const SequenceID& sequenceID);

    PlayJobStatus play(const ParseID& parseID, const EngineJobSourceID& source);

    AddSequenceStatus addSequence(const std::shared_ptr<Sequence>& sequence, const EngineJobSourceID& source);
    void closeSequence(const SequenceID& seqid);
    void cancelSequence(const SequenceID& seqid);

    EngineJobStatus getStatus(const ParseID& pid);
    EngineJobStatus getStatus(const ShotID& sid);
    EngineJobStatus getStatus(const SequenceID& seqID);

    bool getDependencyParser(std::shared_ptr<EventEngineDependencyParser>& dependencyParser);

    bool getJob(const EngineJobID& id, std::shared_ptr<EventEngineJob>& job) const;
    void addJob(const std::shared_ptr<EventEngineJob>& newJob);
    void cancelJob(const EngineJobID& jobID);
    void jobComplete(const EngineJobID& jobID);
    void cancelAll();
    void stopAll();
    void clearAll();

    std::set<EngineJobID> getJobIDs(const EventEngineJobList& jobListType) const;
    std::vector<std::shared_ptr<EventEngineJob>> getJobs(const EventEngineJobList& jobListType) const;

    std::shared_ptr<Shot> createShot(const ShotConfig& shotConfig, const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup);

    void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory);
 
    void addEngine(const EngineID& engineID, DeviceEventParser* deviceParser, EngineTriggerTarget* triggerTarget);

    void getEngineIDs(std::set<EngineID>& engineIDs) const;
    EngineState getEngineState(const EngineID& engineID) const;

    void clearEngine(const EngineID& engineID);
    void stopEngine(const EngineID& engineID);

    void getEngineStates(std::map<EngineID, EngineState>& engineStates) const;

    void setEngineConflictPolicy(const std::shared_ptr<EngineConflictPolicy>& policy);

    bool getParseResult(const ParseID& parseID, std::shared_ptr<ParseResult>& parseResult) const;
    bool getShotResult(const ShotID& shotID, std::shared_ptr<ShotResult>& shotResult) const;

    std::shared_ptr<STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>> getMessageListener() const
    {
        return engineSchedulerMessageListenerDelegate;
    }

    //reserve is only called on other devices.  The main device just calls engine->parse when job is activated.  Inside the main engine, the 
    //device references are used to call schedulder->reserveParse on all owned devices.  Callback messages are directed to the main device engine.
    //Callbacks contain engine references for all top level (owned) devices. When all are reserved, the main engine calls engine->parse on all devices.    
//    void reserveParse(const ParseID& parseID, ParseTree tree);          //local, when ready; create global dependency graph, reserve parse on all dependents
//    void reservePlay(ShotID shotID);       //just waits for engine reservations down the chain, then calls engine->play
    
    void parseJob(const std::shared_ptr<EventEngineJob>& job);

    static void definePlayMessageIDs();
    static const std::map<std::string, unsigned>& getPlayMessageIDs();

private:

    static std::map<std::string, unsigned> playMessageIDs;

    void addSequenceJob(const std::shared_ptr<SequenceJob>& job);
    void refreshSequenceJobs();


    // void play(const ShotID& shotID, const std::shared_ptr<Shot>& shot);
    void stop();

    void findEventTargets(const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup, std::set<STI::Device::DeviceID>& eventTargets);
    void transferTimingFiles(StackTraceData& stackTraceData, STI::Utils::FileServer& remoteFileSever, STI::Utils::VirtualFileServer& targetFileServer);
   
    void assignJobs();
    void assignPlayJobs(const std::set<EngineJobID>& queuedJobIDs, std::set<EngineID>& freeEngines);
    void assignParseJobs(const std::set<EngineJobID>& queuedJobIDs, std::set<EngineID>& freeEngines);

    bool assignJob(const EngineJobID& jobID, const EngineID& engineID);
    void _cancelJob(const EngineJobID& jobID);
    bool isCanceledJob(const STI::Engine::ParseID& parseID);
    bool satisfiesConflictPolicy(const std::shared_ptr<EventEngineJob>& runningJob, const EventEngineJobType& newJobType, const EngineID& engineID);
    void unloadEngines(const EventEngineJobType& type, const EngineID& engineID);

    bool findJob(const ParseID& parseID, std::shared_ptr<EventEngineJob>& job) const;
    bool findJob(const ShotID& shotID, std::shared_ptr<EventEngineJob>& job) const;
    bool findJob(const SequenceID& seqID, std::shared_ptr<SequenceJob>& job) const;

    bool findOldestParsedEngine(std::set<EngineID>& freeEngines, EngineID& engineID);
    bool findParsedEngine(const STI::Engine::ParseID& parseID, std::set<EngineID>& freeEngines, EngineID& engineID);
    bool findRunningEngine(const ShotID& shotID, std::shared_ptr<LocalEventEngine>& engine) const;
    bool findCompletedEngine(const ShotID& shotID, std::shared_ptr<LocalEventEngine>& engine) const;
    
    int getTargetPool(const EngineJobID& jobID);
    bool getParsedEngine(const ParseID& parseID, std::shared_ptr<LocalEventEngine>& engine) const;
    bool getManager(const EngineJobID& jobID, std::shared_ptr<EventEngineManager>& manager);
    bool getManager(const std::shared_ptr<EventEngineJob>& job, std::shared_ptr<EventEngineManager>& manager);

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

    STI::Device::DeviceID localDeviceID;

    std::shared_ptr<LocalEventEngineDependencyParser> localDependencyParser;
	std::shared_ptr<STI::Engine::EventEngineFactory> eventEngineFactory;
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
    std::shared_ptr<EngineConflictPolicy> conflictPolicy;

    STI::Utils::SynchronizedMap<EngineID, std::shared_ptr<EventEngineManager>> engineManagers;  

    STI::Utils::SynchronizedMap<EngineJobID, std::shared_ptr<EventEngineJob>> queuedJobs;
    STI::Utils::SynchronizedMap<EngineJobID, std::shared_ptr<EventEngineJob>> runningJobs;
    STI::Utils::OrderedBufferMap<EngineJobID, std::shared_ptr<EventEngineJob>> completedParseJobs;
    STI::Utils::OrderedBufferMap<EngineJobID, std::shared_ptr<EventEngineJob>> completedPlayJobs;
    
    STI::Utils::SynchronizedMap<EngineJobID, std::shared_ptr<SequenceJob>> queuedSequenceJobs;
    std::shared_ptr<SequenceJob> currentSequenceJob;
    STI::Utils::OrderedBufferMap<EngineJobID, std::shared_ptr<SequenceJob>> completedSequenceJobs;

    bool running;
    std::thread schedulerThread;

    mutable std::mutex jobMutex;
    mutable std::condition_variable jobCondition;

    mutable std::mutex parseResultMutex;
    mutable bool searchingParseResult;

    mutable std::mutex shotResultMutex;
    mutable bool searchingShotResult;


    class EngineSchedulerMessageListenerDelegate : public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
    {
    public:

        EngineSchedulerMessageListenerDelegate(LocalEventEngineScheduler* scheduler) : scheduler(scheduler) {}

        void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
        {
            scheduler->handleMessage(mess);
        }

    private:

        LocalEventEngineScheduler* scheduler;
    };

    std::shared_ptr<EngineSchedulerMessageListenerDelegate> engineSchedulerMessageListenerDelegate;

};

} //Engine
} //STI

#endif
