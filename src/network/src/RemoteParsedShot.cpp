
#include "RemoteParsedShot.h"

#include "Convert_EventEngine.h"
#include "RawEvent.h"

#include <memory>
#include <vector>

using STI::Network::RemoteParsedShot;


RemoteParsedShot::RemoteParsedShot(::STI::TNetwork::TParsedShot_ptr shot)
	: _tShot(STI::TNetwork::TParsedShot::_duplicate(shot))
{
}

RemoteParsedShot::~RemoteParsedShot()
{
}

void RemoteParsedShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events)
{

//	STI::TNetwork::TRawEventSeq_var tEvents(new STI::TNetwork::TRawEventSeq);
	STI::TNetwork::TRawEventSeq_var tEvents;

    events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

	try {
		
        _tShot->getEvents(tEvents); 	//remote call

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
