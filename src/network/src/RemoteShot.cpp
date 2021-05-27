
#include "RemoteShot.h"

#include "Convert_EventEngine.h"
#include "RawEvent.h"

#include <memory>
#include <vector>

using STI::Network::RemoteShot;


RemoteShot::RemoteShot(::STI::TNetwork::TShot_ptr shot)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TShot>(shot, shotMutex)
//	: _tShot(STI::TNetwork::TShot::_duplicate(shot))
{
}

RemoteShot::~RemoteShot()
{
	disable();
}

void RemoteShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	STI::TNetwork::TRawEventSeq_var tEvents;

    events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

	if (isDisabled()) return;

	try {

        getTRef()->getEvents(tEvents); 	//remote call

		if (events != 0) {
			convert<STI::TNetwork::TRawEvent, STI::Engine::RawEvent>(tEvents, *events);
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
