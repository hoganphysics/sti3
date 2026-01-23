#include "RemoteShot.h"

#include "convert/Convert_EventEngine.h"
#include "convert/Convert_ShotResult.h"
#include "convert/Convert_RawEventGroup.h"

#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/ParseResult.h>

#include <memory>
#include <vector>

using STI::Network::RemoteShot;
using STI::Engine::ShotConfig;


RemoteShot::RemoteShot(const STI::Engine::ShotConfig& shotConfig, ::STI::TNetwork::TShotCallback_var shotCallback)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TShotCallback>(shotCallback), shotConfig(shotConfig)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	refreshRequired = true;
}

RemoteShot::~RemoteShot()
{
	disable();
}

bool RemoteShot::getTShotReference(STI::TNetwork::TShotCallback_var& tShotCallback)
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

void RemoteShot::getRootEventGroup(std::shared_ptr<STI::Engine::RawEventGroup>& rootGroup)
{
	refresh();

	std::unique_lock<std::mutex> shotLock(shotMutex);
	rootGroup = rootEventGroup;
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
		// disable(shotLock);
	}
}

bool RemoteShot::refreshEvents()
{
	//has lock
	bool success = false;

	if (isDisabled()) return false;

	rootEventGroup = std::make_shared<STI::Engine::RawEventGroup>();

	STI::TNetwork::TRawEventGroup_var tEventGroup;

	try {

        getTRef()->getRootEventGroup(tEventGroup); 	//remote call

		success = true;

		convert<STI::TNetwork::TRawEventGroup, std::shared_ptr<STI::Engine::RawEventGroup>>(tEventGroup, rootEventGroup);
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
