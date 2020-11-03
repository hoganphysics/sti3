#ifndef STI_ENGINE_EVENTENGINEMANAGER_H
#define STI_ENGINE_EVENTENGINEMANAGER_H

#include "LocalEventEngine.h"
#include "EngineID.h"
#include "DeviceEvent.h"

#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>


namespace STI
{
namespace Engine
{

class LocalEventEngineScheduler;
class EventEngineJob;
class ParseID;


class EventEngineManager
{
public:

    EventEngineManager(const EngineID& engineID, std::shared_ptr<LocalEventEngine> engine, LocalEventEngineScheduler* scheduler);
    ~EventEngineManager();

    bool submitJob(const std::shared_ptr<EventEngineJob>& job);
    bool getJob(std::shared_ptr<EventEngineJob>& job);

    bool jobRunning();
    void abortJob();
    
    void handleParseMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& evt);
    void handlePlayMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& evt);

    bool isParsed(const ParseID& parseID);
    const ParseID& getLastParseID();

private:

    void runJob();

    LocalEventEngineScheduler* scheduler;

    EngineID engineID;
    std::shared_ptr<LocalEventEngine> engine;

	std::shared_ptr<EventEngineJob> currentJob;

	std::thread jobThread;
	bool running;

	mutable std::mutex jobMutex;
	mutable std::condition_variable condition;

};


} //Engine
} //STI

#endif
