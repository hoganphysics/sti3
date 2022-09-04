#include "LocalShot.h"

#include <sti/engine/RawEvent.h>

#include "RawEventGroup.h"

using STI::Engine::LocalShot;
using STI::Engine::ShotConfig;
using STI::Engine::RawEventGroup;


LocalShot::LocalShot(const ShotConfig& config, const std::shared_ptr<RawEventGroup>& baseGroup)
: shotConfig(config), baseEventGroup(baseGroup)
{
}

LocalShot::~LocalShot()
{
}

const ShotConfig& LocalShot::getShotConfig() const
{
    return shotConfig;
}

void LocalShot::getBaseEventGroup(std::shared_ptr<RawEventGroup>& baseGroup)
{
    baseGroup = baseEventGroup;
}

void LocalShot::setBaseEventGroup(const std::shared_ptr<RawEventGroup>& baseGroup)
{
    baseEventGroup = baseGroup;
}
