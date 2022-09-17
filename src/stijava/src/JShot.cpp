#include "JShot.h"

#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/ShotConfig.h>

using STI::Engine::JShot;
using STI::Engine::ShotConfig;
using STI::Engine::RawEventGroup;


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

std::shared_ptr<RawEventGroup> JShot::getRootEventGroup()
{
    auto events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

    std::shared_ptr<RawEventGroup> rootGroup;
    getRootEventGroup(rootGroup);

    if (rootGroup != 0) {
        return rootGroup;
    }

    rootGroup = std::make_shared<RawEventGroup>();
    return rootGroup;
}

void JShot::getRootEventGroup(std::shared_ptr<RawEventGroup>& rootGroup)
{
    if (shot_ != 0) {
        shot_->getRootEventGroup(rootGroup);
    }
}

