
#ifndef STI_PYTHON_EVENTENGINESCHEDULERPY_H
#define STI_PYTHON_EVENTENGINESCHEDULERPY_H

#include <sti/engine/EventEngineScheduler.h>

#include <memory>


namespace STI
{
namespace Python
{

class LocalShotPy;


class EventEngineSchedulerPy
{
public:

    EventEngineSchedulerPy(const std::shared_ptr<STI::Engine::EventEngineScheduler>& engineScheduler);
    virtual ~EventEngineSchedulerPy();

    STI::Engine::ParseID parse(const std::shared_ptr<LocalShotPy>& shot);
    STI::Engine::ShotID play(const STI::Engine::ParseID& parseID, const STI::Engine::EngineJobSourceID& source);

    STI::Engine::EngineJobStatus getStatus(const STI::Engine::ParseID& pid);
    STI::Engine::EngineJobStatus getStatus(const STI::Engine::ShotID& sid);

    void cancelJob(const STI::Engine::EngineJobID& jobID);
    void cancelAll();

    std::set<STI::Engine::EngineJobID> getQueuedJobs() const;
    std::set<STI::Engine::EngineJobID> getRunningJobs() const;
    std::set<STI::Engine::EngineJobID> getCompletedJobs() const;

    // void parse(const STI::Engine::ParseID& parseID, const std::shared_ptr<STI::Engine::ParsedShot>& shot);
    // void play(const STI::Engine::ShotID& shotID);
    // void cancelJob(const STI::Engine::EngineJobID& jobID);

private:

    std::shared_ptr<STI::Engine::EventEngineScheduler> engineScheduler;

};


} //Python
} //STI

#endif

