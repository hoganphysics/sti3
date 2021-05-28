
#include "RemoteShot.h"

#include "Convert_EventEngine.h"
#include "RawEvent.h"

#include <memory>
#include <vector>

#include <iostream>

using STI::Network::RemoteShot;


RemoteShot::RemoteShot(::STI::TNetwork::TShot_ptr shot)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TShot>(shot, shotMutex)
//	: _tShot(STI::TNetwork::TShot::_duplicate(shot))
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	refreshRequired = true;

	std::cout << "create RemoteShot()" << std::endl;

}

RemoteShot::~RemoteShot()
{
	disable();
	std::cout << "~RemoteShot()" << std::endl;
}

void RemoteShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	if (refreshRequired) {
		refreshEvents();

		//once events have been received, release remote reference
		if (!refreshRequired) {
			disable(shotLock);
		}
	}

	events = storedEvents;
}

void RemoteShot::refreshEvents()
{
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
