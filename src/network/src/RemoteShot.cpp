
#include "RemoteShot.h"

#include "Convert_EventEngine.h"
#include "RawEvent.h"

#include <memory>
#include <vector>

#include <iostream>

using STI::Network::RemoteShot;
using STI::Engine::ShotConfig;


RemoteShot::RemoteShot(const STI::Engine::ShotConfig& shotConfig, ::STI::TNetwork::TShotEventsCallback_ptr shotCallback)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TShotEventsCallback>(shotCallback, shotMutex), shotConfig(shotConfig)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	refreshRequired = true;
}

RemoteShot::~RemoteShot()
{
	disable();
}

bool RemoteShot::getTShotReference(STI::TNetwork::TShotEventsCallback_ptr& tShotCallback)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	if (isDisabled()) return false;

	tShotCallback = STI::TNetwork::TShotEventsCallback::_duplicate(getTRef());

	return !CORBA::is_nil(tShotCallback);
}

const ShotConfig& RemoteShot::getShotConfig() const
{
	return shotConfig;
}

void RemoteShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	if (refreshRequired) {
		_refreshEvents();

		//once events have been received, release remote reference
		if (!refreshRequired) {
			disable(shotLock);
		}
	}

	events = storedEvents;
}

void RemoteShot::_refreshEvents()
{
	//has lock

	if (isDisabled()) return;

	storedEvents = std::make_shared<std::vector<STI::Engine::RawEvent>>();

	STI::TNetwork::TRawEventSeq_var tEvents;

	try {

        getTRef()->getEvents(tEvents); 	//remote call

		refreshRequired = false;

		if (storedEvents != 0) {
			convert<STI::TNetwork::TRawEvent, STI::Engine::RawEvent>(tEvents, *storedEvents);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}
