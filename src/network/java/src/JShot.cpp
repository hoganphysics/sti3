
#include "JShot.h"
#include "RawEvent.h"
#include "ShotConfig.h"

using STI::Engine::JShot;
using STI::Engine::ShotConfig;


JShot::JShot(std::shared_ptr<STI::Engine::Shot>& shot)
: shot_(shot)
{
    if (shot != 0) {
        shotConfig = shot->getShotConfig();
    }
}

JShot::~JShot()
{
}

const ShotConfig& JShot::getShotConfig() const
{
    return shotConfig;
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

