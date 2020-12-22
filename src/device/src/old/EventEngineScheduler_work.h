#ifndef STI_ENGINE_EVENTENGINESCHEDULER_H
#define STI_ENGINE_EVENTENGINESCHEDULER_H

#include "ParseID.h"
#include "EventEngine.h"
#include "Device.h"
#include "DeviceCollection.h"

//#include <queue>

#include <list>
#include <map>


/*
Implements prioritized parallel distributed scheduling:
- Prioritized: Each job has an absolute priority rank and jobs proceed in order.
- Parallel: Multiple jobs can be run in parallel, depending on overlap restrictions of the specific jobs.
- Distributed: Jobs are run synchronously across the network, without global network information (only local knowledge at each node).
- Scheduling: Jobs (play/parse) are added to a queue and run when required resources are available, based on priority.
*/

namespace STI
{
namespace Engine
{

class EngineTask
{
    ShotID id;

    set<DeviceID> dependentDevices; //graph?

};

class EnginePlayTask : public EngineTask
{
    EnginePlayTask(EngineTask parseTask);
    //This could be responsible for dividing event list and recruiting events engines...
    //Shouldn't the Engine know it's dependencies though?
    

};

class EventEngineArbiter
{

};

class EngineResource
{
    EventEngine* engine;

};

typedef unsigned EngineID;

//linked list of these:
class EngineReservation
{
    ParseID parseID;
    EngineID engineID;

//    bool isParseReady();
//    bool isPlayReady();
};

class EventEnginePool
{
public:

    //pool size

    void getNextAvailableEngine(EventEngine* engine);


private:

    std::vector<EventEngine> enginePool;
};

//For transmitting to client to show when engines are doing
//Somehow need a group ID field so devices can be grouped under servers.
class Job
{
    enum class JobType { Parse, Play };
    ParseID pid;
    ShotID sid;  //for play jobs -- new two Job classes?
    DeviceID deviceID;
    EngineID engineID;
};

class EngineJobID
{
public:

    enum class EngineJobType { Parse, Play };
    
    bool operator<(const EngineJobID& rhs);
    
    EngineJobType type;
    ParseID pid;
    ShotID sid;  //for play jobs -- new two Job classes?
};


class ParsedDevice
{
    bool isAbstract();
    std::string id();

    STI::Device::DeviceID id;
};

class ParsedChannel
{
    ParsedDevice device;

    bool isAbstract();
    std::string id();       //key for lookup in channel definitions dictionary
};

class ParsedEvent
{
    //time, value
    ParsedChannel channel;
    //stack trace
};

class ParsedShot
{
    std::vector<ParsedEvent> events;
    //files
};

class ParseJob : public STI::Utils::Job<EngineJobID>
{
public:

    ParseID id;

    Engine* engine;

    ParsedShot shot;

    STI::Device::DeviceID jobOwner;

    //Devide targetIDs into categories based on their target server
    std::set<STI::Device::DeviceID> localTargetIDs;     //targets that have THIS device as their server
    std::map<STI::Device::DeviceID, std::set<STI::Device::DeviceID>> remoteTargetIDs;   // targets owned by devices this device owns; map: (locally owned ID) -> remoteTargetIDs
    std::set<STI::Device::DeviceID> missingTargetIDs;
};

class PlayJob : public STI::Utils::Job<EngineJobID>
{
    Engine* engine;
};


/*
Rough idea:

- scehduler has a list of EngineJobQueues. Submits ParseJob.
- ParseJob holds submitted events and ParseID
- When ParseJob comes up to run, it calls a function in scheduler with it's Job info
- This way the scheduler can listen for events from partners, and contact partners with its device reference (rather than making the job or queue do this)
- But: the queue has a list of EngineJobs, not ParseJobs or PlayJobs, so there is slicing.
- Options: dynamic_cast, store two sets? Or:
    - Queues could just store the JobID, not the job itself. The scheduler stores parse jobs and play jobs in separate lists, avoiding slicing.

*/
//

//Running these in a thread is an implementation detail on the local scehduler.
//Interface of scheduler needs to expose all EventEngine functionality (not necessarily all functions!)
//Perhaps this Manager class in just a thread wrapper.
//Should the EventEngine call parse() on dependent devices, or should the schduler do that?
//Who calls trigger?
//Who collects results?  Messages?
class EventEngineManager
{
    void parse(ParseJob job);
    void play(PlayJob job);

    bool running;
    std::thread engineThread;

    EventEngine engine;
    STI::Device::Device* localDevice;
};

STI::Utils::SynchronizedMap<EngineJobID, ParseJob> parseJobs;
STI::Utils::SynchronizedMap<EngineJobID, PlayJob> playJobs;
STI::Utils::SynchronizedMap<EngineID, EventEngineManager> engines;


class EventEngineScheduler
{
private:
    STI::Device::Device* localDevice;

public:
    //need a UI level function that get's called by the client, and is reponsible for parsing other devices in the network
    //ParsedChannels contains all the absract channel definitions, for all event (can be redunant if the same event goes to multiple sub servers)
    //Maybe better to accept a list of (events, channels) with as many sets of channel targets as needed?  Maybe can do both.
    
    //Consider alternative: JobID
//    void parse(ParseID id, const STI::Engine::RawEventVector& events, std::vector<ParsedChannel> channels)
    void parse(ParseID parseID, const ParsedShot& shot)
    {
        //split events by deviceID
        // -- For all concrete channel events, add to map
        // -- For abstract channels events, find all rules in channels and add to the map as a concrete (possibly by reference)

        // Splitting results in a list of DeviceID targets and associated events, including self.
        // Map network event targets from this node (NodeWalker)
        // Store event target network in a map associated with this ParseID.  This will be used for status checking and data saving.

        // reserveParse for self


        //reserve parse on deviceIDs:
        // -any device listed in any explicit event in the shot (following abstract lookups)
        // -any device that THIS device has declared as an event target, IF this device have explicit events

        //should add a Job to the local priority queue
        reserveParse(parseID, deviceIDs);    //deviceID = above
    }

    bool onTargetServerPath(const STI::Device::DeviceID& targetServerID)
    {

        if(targetServerID == localDevice->getID()) {
            return true;
        }
        
        std::shared_ptr<STI::Device::DeviceCollection> localCollection;
        localDevice->getCollection(localCollection);

        //check any devices in the local collection to see if they are the server target
        if(localCollection->contains(targetServerID)) {
            return true;
        }

        //pass it down the network; if any device is the target server, return true
        std::set<STI::Device::DeviceID> ids;
        std::shared_ptr<STI::Device::Device> device;
        std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
        localCollection->getIDs(ids);

        bool found = false;

        for(auto& id : ids) {
            if(localCollection->get(id, device) && device != 0 && 
            device->getEngineScheduler(scheduler) && scheduler != 0 && 
            scheduler->onTargetServerPath(targetServerID)) {
                found = true;
                break;
            }
        }

        return found;

    }

    bool onTargetServerPath(const STI::Device::DeviceID& targetServerID, const STI::Device::DeviceID& managedID) 
    {

        std::shared_ptr<STI::Device::DeviceCollection> localCollection;
        localDevice->getCollection(localCollection);
        
        std::shared_ptr<STI::Device::Device> device;
        std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
        
        return localCollection->get(managedID, device) && device != 0 && 
            device->getEngineScheduler(scheduler) && scheduler != 0 && 
            scheduler->onTargetServerPath(targetServerID);
    }

    void reserveParse(ParseID parseID, const std::set<STI::Device::DeviceID>& targetIDs)
    {
        //If reserved already for this id, return (catches partners being called by the server and a device), and redundant parses.
        // Select an engine and add id to parseReservations for that engine, sorted by timestamp
        // call reserveParse on all direct event targets

        std::shared_ptr<STI::Device::DeviceCollection> localCollection;
        localDevice->getCollection(localCollection);

        //Devide targetIDs into categories based on their target server
        std::unique_ptr<ParseJob> parseJob = std::make_unique<ParseJob>();

        std::set<STI::Device::DeviceID> localTargetIDs;     //targets that have THIS device as their server
        std::map<STI::Device::DeviceID, std::set<STI::Device::DeviceID>> remoteTargetIDs;   // targets owned by devices this device owns; map: (locally owned ID) -> remoteTargetIDs
        std::set<STI::Device::DeviceID> missingTargetIDs;

        for(auto& id : targetIDs) {

            STI::Device::DeviceID targetServerID;
            STI::Device::DeviceID::stringToDeviceID(id.getTargetServerID(), targetServerID);

            if(targetServerID == localDevice->getID()) {
                //This device is the target device's server
                //Add to list of devices that this device manages
                localTargetIDs.insert(id);
            }
            else if( localCollection->contains(targetServerID) ) {
                //This device has direct control of the the server of the target device id.
                //Add this id to the list of devices managed by the child device:
                remoteTargetIDs[targetServerID].insert(id);
            }
            else {
                //pass this id down the network to find the correct delegate
//                onTargetServerPath(targetServerID);
                std::set<STI::Device::DeviceID> managedIDs;
                localCollection->getIDs(managedIDs);

                bool found = false;

                for(auto& mID : managedIDs) {
                    if( onTargetServerPath(targetServerID, mID) ) {
                        //The target server is managed somewhere in the network under mID
                        remoteTargetIDs[mID].insert(id);
                        found = true;
                        break;
                    }
                }

                if(!found) {
                    //error; this id is not managed by any device on the network reachable from this device
                    //Throw error
                    missingTargetIDs.insert(id);
                    //return;
                }
            }
        }

        //All device ids in the input list have been divided

        if(missingTargetIDs.size() > 0) {
            //throw error: some devices not found!
            return;
        }

        std::set<STI::Device::DeviceID> ids;
        localCollection->getIDs(ids);

        std::shared_ptr<STI::Device::Device> device;
        std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

        for(auto& id : ids) {
            if(localCollection->get(id, device) && device != 0 && 
            device->getEngineScheduler(scheduler) && scheduler != 0) {
                 scheduler->reserveParse(parseID, remoteTargetIDs[id]);     //should add jobs to remote priority queues 
            } 
        }
    }

    bool parse(ParseID id, const STI::Engine::RawEventVector& events)
    {
        // Mutex lock
         
        //Make sure id is next up in parseReservations for its engine
        //Make sure that its engine is cleared to parse
        // -- Does the engine have a play request in playReservations that does NOT match id?
        // -- Does the engine have owned data?

        //New thread:
        engine->parse(...);
    }

    // //Interface: entry point for clients
    // void play(ParseID id)
    // {
    //     //Make sure id has been parsed by some engine (happend in reservePlay)
    //     //perhaps these two can be joined...

    //     // reservePlay for self
    // }

    //Actually, this IS the interface.  Should call play reserve.
    //The idea is that play show just add to a job list and then return (assuming the shot has been parsed.)
    bool play(ParseID id, ShotID& shot)
    {
        //should probably merge with reservePlay

        //assign shot timestamp


//Actually need a play queue that calls this part.  EventQueue class should work. (Or maybe not, since we want control of inserting plays arbitarily to maintain TimeStamp sorting)
        //New thread:
        engine->play(...);

        // call play on first-level event targets
    }


    bool reservePlay(ParseID id)
    {
        // Mutex lock

        auto engineIt = parsedShots.find(id);
        if(engineIt == parsedShots.end()) {
            //not currently parsed in any engine
            return false;
        }


        //Make sure engine actually has engine->lastParseID == id

        // call reservePlay on all direct event targets

        //If success, insert id into playReservations, sorted by TimeStamp


    }

    bool isParsed(ParseID id);
    bool isPlayed(ShotID shot);

    void getMeasurements(std::shared_ptr<MeasurementVector> results);
    void release(ShotID shot);      //used by a remote device to take ownership of the results of this shot, freeing up the local engine to delete as needed.
    
    void getJobs();

    std::map<ParseID, EngineID> parsedShots;
    std::list<ParseID> playReservations;

    //Interface?  scheduleParse(...)
//    bool parse(...);
//    bool play(...);

//    bool parseParametric(ParseID id, const STI::Engine::RawEventVector& events, vector<ShotVars> overwritten);//attempts to parse events ONCE and then modify events (value, MAYBE times) using something like DynamicValue
//    bool playParametric(ParseID id, vector<ShotVars> overwritten, vector<ShotVars> overwritten);//attempts to parse events ONCE and then modify events (value, MAYBE times) using something like DynamicValue

    //Sequences?  Parse/play multiple shots in a row, given a list of overridden vars.  Scheduler takes care of load balancing automatically, preserving order of play.
    //Not possible without built in python interpreter -- must reparse.  Add this to the python lib? Requires client to be live...
    //No, just requires that the lib user runs a python script; this can certainly continue running even if the user logs off.
    //The task list just needs to include the submitter information and the interface must allow for jobs to be killed.
    //Recommendation is that sequence parsing be off loaded to the python library -- user can submit a sequence play and it will 
    //interact with the server to submit tasks until complete.  Do need a way to group these -- Common ShotID?
//    bool parseSequence(); //not recommneded.

    //Or maybe, could allow a parseSequence() function to tell the scheduler that there is another shot in the same sequence.



//    bool addParseTask(ParseID id, const STI::Engine::RawEventVector& events, ShotID& shot);      //py lib should block if this returns false, and wait until resources are available
//    bool addPlayTask(ShotID shot, ResultsTicket& ticket);   //or EngineResultTicket

    //void wait();  //some way for callers to block waiting for addParseTask to be ready.
    //Could have caller add itself as event listener, and this device pushes 'buffer available' event

//    bool addTask(EngineTask task, EngineTicket& ticket);
//    void getTasks(tasklist);

private:

    //std::queue<EngineTask> tasks;
    //maybe store task in a dependency graph?
    //Then the scheduler can load balance the engines by counting dependencies of the nodes


};



class MultipleJobs
{

    //STI::Utils::SynchronizedMap<EngineJobID, ParseJob> parseJobs;
    //STI::Utils::SynchronizedMap<EngineJobID, PlayJob> playJobs;

    //submitted jobs
    std::map<EngineJobID, ParseJob> parseJobs;
    std::map<EngineJobID, PlayJob> playJobs;

    std::set<EngineJobID> runningJobs;
    STI::Utils::OrderedBufferMap<EngineJobID, EngineJob> completedJobs;     //circular buffer of recently finished jobs

    std::map<EngineID, EngineJobID> engineAssignments;


    STI::Utils::SynchronizedMap<EngineJobID, EngineJob> queuedJobs;
    STI::Utils::SynchronizedMap<EngineJobID, EngineJob> runningJobs;
    STI::Utils::SynchronizedMap<EngineID, EngineJobID> engineAssignments;
    STI::Utils::OrderedBufferMap<EngineJobID, EngineJob> completedJobs;

    void assignJobs()
    {
        EventEngine* engine;

        //an event has finished;
        //mutex lock

        //Is any engine currently playing?
        for(all runningJobs) {
            if(playing = job.type == Play) break;
        }

        std::set<EngineID> engineIDs;
        getIdleEngines(engineIDs);    //using runningJobs

        //attempt play
        if(!playing) {
        
            for(auto& playjob : playJobs) {    //sorted in order of timestamp
                //Is this play parsed by one of the idle engines?
                for(auto& id : engineIDs) {
                    
                    if(getEngine(id)->isParsedFor(playjob->first.pid)) {
                        engineAssignments[id] = playjob->first;
                        runningJobs.insert(playjob->first);
                        playing = true;
                        break;
                    }
                }
                if (playing) break;
            }
        }

        getIdleEngines(engineIDs);

        //Assign parse jobs to any remaining idle engines
        for(auto& id : engineIDs) {

            //make sure this engine is not parsed for any parse is in the play queue...
            //this is where managing the queues in there own instance really helps
            // if(addJob(parseJob)) ...

            playJobs.contains(id)
        }





    }

    void wait()
    {
        refresh();  //get any new jobs

        while(no job && running) {
            condition->wait();
        }
    }

    void eventHandlerLoop()
    {
        EngineJob job;

        while (running)
        {
            wait();

            //...
            EngineJobID jobid = engineAssignments[thisEngineID];

            auto playit = playJobs.find(jobid);
            if(playit == playJobs.end()) {
                auto parseit = parseJobs.find(jobid);
                if(parseit == parseJobs.end()) {
                    job = *parseit;
                }
            }
            else {
                job = *playit;
            }

            completedJobs.add(jobid, job);

            runningJobs.erase(jobid);
            
        }
    }

};



// class ParseJob
// {
// public:

//     ParseID id;
//     ParsedShot shot;

//     STI::Device::DeviceID jobOwner;

//     //Devide targetIDs into categories based on their target server
//     std::set<STI::Device::DeviceID> localTargetIDs;     //targets that have THIS device as their server
//     std::map<STI::Device::DeviceID, std::set<STI::Device::DeviceID>> remoteTargetIDs;   // targets owned by devices this device owns; map: (locally owned ID) -> remoteTargetIDs
//     std::set<STI::Device::DeviceID> missingTargetIDs;
// };

// class PlayJob
// {
// public:
    
//     ShotID id;
// };




class EventEngineJobManager
{
    bool submitJob(EventEngineJob job);
    bool jobRunning();
    void abortJob();

private:
    EventEngineScheduler* scheduler;

    EventEngine* engine;

	std::deque<EventEngineJob> fifo;
	std::thread jobThread;
	bool running;

	mutable std::mutex fifoMutex;
	mutable std::condition_variable condition;

public:

EventEngineJobManager() : running(false)
{
}

~EventEngineJobManager() 
{
	stop();
}

void start()
{
	std::unique_lock<std::mutex> writeLock(fifoMutex);
	if (!running) {
		running = true;
		jobThread = std::thread(&EventEngineJobManager::eventHandlerLoop, this);
	}
}

void stop()
{
	if (!running)
		return;

	{
		std::unique_lock<std::mutex> writeLock(fifoMutex);
		running = false;
		condition.notify_all();
	}
	jobThread.join();
}

void wait()
{
	std::unique_lock<std::mutex> writeLock(fifoMutex);

	while (!_isNextEventReady() && running) {
		condition.wait(writeLock);
	}
}

void eventHandlerLoop()
{
	EventEngineJob eventToHandle;

	while (running)
	{
		wait();

		{
			std::unique_lock<std::mutex> writeLock(fifoMutex);

			if (!fifo.empty()) {	//just in case, check for events
				eventToHandle = fifo.front();
				fifo.pop_front();
			}
		}

		if (running) {		//check for recent call to stop()
            scheduler->reserveParse(job);
			eventToHandle.run();
		}
	}
}

};




} //Engine
} //STI

#endif
