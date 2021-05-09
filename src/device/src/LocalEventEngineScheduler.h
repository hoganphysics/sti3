#ifndef STI_ENGINE_LOCALEVENTENGINESCHEDULER_H
#define STI_ENGINE_LOCALEVENTENGINESCHEDULER_H

#include "EventEngineScheduler.h"

#include "ParseID.h"
#include "LocalEventEngine.h"
#include "Device.h"
#include "DeviceCollection.h"

#include "EngineJobID.h"

#include "DeviceMessage.h"
#include "DeviceMessageListener.h"

#include "Shot.h"
#include "SynchronizedMap.h"
#include "utils/OrderedBufferMap.h"

#include "LocalDevice.h"
#include "EngineID.h"
#include "EventEngineDependencyTree.h"
#include "DeviceTrace.h"

//#include <queue>

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

class EventEngineManager;
class EventEngineJob;
class EngineJobID;
class LocalEventEngine;

class ParseID;
class Shot;
class EventEngineFactory;

class LocalEventEngineScheduler : public EventEngineScheduler, 
                                  public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
{
public:
    
    // LocalEventEngineScheduler(const STI::Device::DeviceID& localDeviceID, const std::shared_ptr<STI::Device::DeviceCollection>& localCollection);
    LocalEventEngineScheduler(STI::Device::LocalDevice* localDevice, const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory);
    ~LocalEventEngineScheduler();

    //local interface (called from python, for example)
    void parse(const ParseID& parseID, const std::shared_ptr<Shot>& shot);        //local; add event to queue
    void play(const ShotID& shotID);

    void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                        std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages, 
                        const STI::Device::DeviceTrace& trace);
    
    void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                        std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages, 
                        unsigned maxRecursions);
    void addDeviceEventTargets(EventEngineDependencyTree& tree, std::vector<EngineParsingMessage>& messages, const STI::Device::DeviceTrace& trace);

    void addJob(const std::shared_ptr<EventEngineJob>& newJob);
    void cancelJob(const EngineJobID& jobID);
    void jobComplete(const EngineJobID& jobID);


    void cancelAll();
    void stopAll();
    
    // std::shared_ptr<EventEngineJob> createJob(const ParseID& parseID, 
    //                                           const std::shared_ptr<Shot>& shot,
    //                                           const std::shared_ptr<EventEngineDependencyTree>& tree, 
    //                                           const STI::Device::DeviceID& owner, 
    //                                           const std::set<STI::Device::DeviceID>& missingTargets);
    
    std::shared_ptr<Shot> createShot(const std::shared_ptr<RawEventVector>& events);

    void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory);
 
    void addEngine(const EngineID& engineID, DeviceEventParser* deviceParser);

    bool getParsedEvents(const ParseID& parseID, DeviceEventMap& events) const;
    bool getParsingMessages(const ParseID& parseID, std::vector<EngineParsingMessage>& messages) const;
    bool getParsedTree(const ParseID& parseID, std::shared_ptr<EventEngineDependencyTree>& tree) const;

    //how to indicate that a "single line timing file" plays on engine 0?
    //how to customize engine behavior per engine, (e.g., allocate memory ranges for FPGA)
    //FPGA_EngineManager

    /*
    Engine queue selection
    - If play event, and one of the engines is parsed for that ID (or has parse ID in its queue) add to this engine
    - If play event and not found, reject
    - If parse event, add to engine with the oldest parse
    */


    //reserve is only called on other devices.  The main device just calls engine->parse when job is activated.  Inside the main engine, the 
    //device references are used to call schedulder->reserveParse on all owned devices.  Callback messages are directed to the main device engine.
    //Callbacks contain engine references for all top level (owned) devices. When all are reserved, the main engine calls engine->parse on all devices.    
//    void reserveParse(const ParseID& parseID, ParseTree tree);          //local, when ready; create global dependency graph, reserve parse on all dependents
    //void parse(const EventEngineJob& job); 

//    void reservePlay(ShotID shotID);       //just waits for engine reservations down the chain, then calls engine->play
    //void play(const EventEngineJob& job); //no need for these here -- do this with direct call to relevant engine, after reserve is successful

private:

    bool loopDetected(const STI::Device::DeviceTrace& trace, STI::Device::DeviceTrace& newTrace);

    void getServerChainIDs(std::set<STI::Device::DeviceID>& serverIDs);
    
    bool getTargetScheduler(const STI::Device::DeviceID& id, std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);

    void addToTargetsByServer(const std::set<STI::Device::DeviceID>& targets, const EventEngineDependencyTree& tree, 
                                std::map<std::string, std::set<STI::Device::DeviceID>>& targetsByServer);
    
    void getDownstreamIDs(const std::map<std::string, std::set<STI::Device::DeviceID>> targetsByServer, const EventEngineDependencyTree& tree, 
                            std::set<STI::Device::DeviceID>& downstreamIDs);


    void getPartnerDeviceDependants(const STI::Device::DeviceID& partnerID, const std::set<STI::Device::DeviceID>& targets, EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingIDs, std::vector<EngineParsingMessage>& messages, const STI::Device::DeviceTrace& trace);

    //void findMissingTarget(const std::set<STI::Device::DeviceID>& missingTargets, EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace);
    
    void assignJobs();
    bool assignJob(const EngineJobID& jobID, const EngineID& engineID);
    void _cancelJob(const EngineJobID& jobID);
    bool isCanceledJob(const STI::Engine::ParseID& parseID);

    bool findParsedEngine(const STI::Engine::ParseID& parseID, std::set<EngineID>& freeEngines, EngineID& engineID);
    bool findOldestParsedEngine(std::set<EngineID>& freeEngines, EngineID& engineID);

    bool getManager(const EngineJobID& jobID, std::shared_ptr<EventEngineManager>& manager);
    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

    bool findJob(const ParseID& parseID, std::shared_ptr<EventEngineJob>& job) const;
    bool getParsedEngine(const ParseID& parseID, std::shared_ptr<LocalEventEngine>& engine) const;

    STI::Device::LocalDevice* localDevice;
    STI::Device::DeviceID localDeviceID;
   std::shared_ptr<STI::Device::DeviceCollection> localCollection;

    STI::Utils::SynchronizedMap<EngineID, std::shared_ptr<EventEngineManager>> engineManagers;

	std::shared_ptr<STI::Engine::EventEngineFactory> eventEngineFactory;

    STI::Utils::SynchronizedMap<EngineJobID, std::shared_ptr<EventEngineJob>> queuedJobs;
    STI::Utils::SynchronizedMap<EngineJobID, std::shared_ptr<EventEngineJob>> runningJobs;
    STI::Utils::OrderedBufferMap<EngineJobID, std::shared_ptr<EventEngineJob>> completedJobs;

    void stop();

    bool running;
    std::thread schedulerThread;

    mutable std::mutex jobMutex;
    mutable std::condition_variable jobCondition;

};


} //Engine
} //STI

#endif
