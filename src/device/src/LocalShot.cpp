#include "LocalShot.h"

#include <sti/engine/RawEvent.h>

#include <sti/engine/RawEventGroup.h>

using STI::Engine::LocalShot;
using STI::Engine::ShotConfig;
using STI::Engine::RawEventGroup;


LocalShot::LocalShot(const ShotConfig& config, const std::shared_ptr<RawEventGroup>& rootGroup)
: shotConfig(config), rootEventGroup(rootGroup)
{
}

LocalShot::~LocalShot()
{
}

const ShotConfig& LocalShot::getShotConfig() const
{
    return shotConfig;
}

void LocalShot::getRootEventGroup(std::shared_ptr<RawEventGroup>& rootGroup)
{
    rootGroup = rootEventGroup;
}

void LocalShot::setRootEventGroup(const std::shared_ptr<RawEventGroup>& rootGroup)
{
    rootEventGroup = rootGroup;
}
