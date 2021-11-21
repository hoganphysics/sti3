
#include "LocalShotPy.h"
#include "RawEvent.h"

using STI::Python::LocalShotPy;



LocalShotPy::LocalShotPy(const STI::Engine::ShotConfig& config)
: STI::Engine::LocalShot(config)
{
}

LocalShotPy::~LocalShotPy()
{
}

std::vector<STI::Engine::RawEvent> LocalShotPy::getEvents()
{
    std::shared_ptr<std::vector<STI::Engine::RawEvent>> events;
    STI::Engine::LocalShot::getEvents(events);

    if (events == 0) {
        events = std::make_shared<std::vector<STI::Engine::RawEvent>>();
    }
    return *events;
}

