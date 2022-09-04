
#include "RemoteShot.h"

#include "Convert_EventEngine.h"
#include "Convert_ShotResult.h"
#include "Convert_RawEventGroup.h"

#include "RawEventGroup.h"

#include <sti/engine/RawEvent.h>
#include <sti/engine/ParseResult.h>

#include <memory>
#include <vector>
// #include <iostream>

using STI::Network::RemoteShot;
using STI::Engine::ShotConfig;


RemoteShot::RemoteShot(const STI::Engine::ShotConfig& shotConfig, ::STI::TNetwork::TShotCallback_ptr shotCallback)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TShotCallback>(shotCallback, shotMutex), shotConfig(shotConfig)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	refreshRequired = true;
}

RemoteShot::~RemoteShot()
{
	disable();
}

bool RemoteShot::getTShotReference(STI::TNetwork::TShotCallback_ptr& tShotCallback)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	if (isDisabled()) return false;

	tShotCallback = STI::TNetwork::TShotCallback::_duplicate(getTRef());

	return !CORBA::is_nil(tShotCallback);
}

const ShotConfig& RemoteShot::getShotConfig() const
{
	return shotConfig;
}

void RemoteShot::getBaseEventGroup(std::shared_ptr<STI::Engine::RawEventGroup>& baseGroup)
{
	refresh();

	std::unique_lock<std::mutex> shotLock(shotMutex);
	baseGroup = baseEventGroup;
}

void RemoteShot::refresh()
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	if (refreshRequired) {
		bool success = true;
		
		success &= refreshEvents();
		refreshRequired = !success;		
	}

	//once events have been received, release remote reference
	if (!refreshRequired) {
		disable(shotLock);
	}
}

bool RemoteShot::refreshEvents()
{
	//has lock
	bool success = false;

	if (isDisabled()) return false;

	baseEventGroup = std::make_shared<STI::Engine::RawEventGroup>();

	STI::TNetwork::TRawEventGroup_var tEventGroup;

	try {

        getTRef()->getBaseEventGroup(tEventGroup); 	//remote call

		success = true;

		convert<STI::TNetwork::TRawEventGroup, std::shared_ptr<STI::Engine::RawEventGroup>>(tEventGroup, baseEventGroup);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}
