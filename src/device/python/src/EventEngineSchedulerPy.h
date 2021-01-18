
#ifndef STI_PYTHON_EVENTENGINESCHEDULERPY_H
#define STI_PYTHON_EVENTENGINESCHEDULERPY_H

#include "EventEngineScheduler.h"

#include <memory>


namespace STI
{
namespace Python
{


class EventEngineSchedulerPy
{
public:

    EventEngineSchedulerPy(const std::shared_ptr<STI::Engine::EventEngineScheduler>& engineScheduler);
    virtual ~EventEngineSchedulerPy();

    // void parse(const STI::Engine::ParseID& parseID, const std::shared_ptr<STI::Engine::ParsedShot>& shot);
    // void play(const STI::Engine::ShotID& shotID);
    // void cancelJob(const STI::Engine::EngineJobID& jobID);

private:

    std::shared_ptr<STI::Engine::EventEngineScheduler> engineScheduler;

};


} //Python
} //STI

#endif

