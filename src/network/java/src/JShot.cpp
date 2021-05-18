
#include "JShot.h"
#include "RawEvent.h"

using STI::Engine::JShot;


JShot::JShot(std::shared_ptr<STI::Engine::Shot>& shot)
: shot_(shot)
{
}

JShot::~JShot()
{
}


std::shared_ptr<std::vector<STI::Engine::RawEvent>> JShot::getEvents()
{
    auto events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

    if (shot_ != 0) {
        shot_->getEvents(events);
    }
    return events;
}

void JShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
{
    evts = getEvents();
}

