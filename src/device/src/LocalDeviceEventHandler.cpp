

#include "LocalDeviceEventHandler.h"
#include "DeviceEvent.h"


using STI::Device::LocalDeviceEventHandler;
using STI::Device::DeviceEvent;


LocalDeviceEventHandler::LocalDeviceEventHandler() : eventQueue(this)
{
	eventQueue.start();
}

LocalDeviceEventHandler::~LocalDeviceEventHandler()
{
}

void LocalDeviceEventHandler::addEvent(const std::shared_ptr<DeviceEvent>& evt)
{
	eventQueue.addEvent(evt);
}

void LocalDeviceEventHandler::clearEvents()
{
	eventQueue.clearEvents();
}


bool LocalDeviceEventHandler::hasListeners(const std::shared_ptr<DeviceEvent>& evt)
{
	auto it = listenersTypes.find(evt->getType());

	return (it != listenersTypes.end() && it->second > 0);
//	return listenersTypes.count(evt->getType()) == 1;
}

void LocalDeviceEventHandler::handleEvent(const std::shared_ptr<DeviceEvent>& evt)
{
	////hasListeners
	//if (!hasListeners(evt)) {
	//	if (genericListeners.size() > 0) {
	//		genericListeners.handleEvent(evt);
	//	}
	//	return;
	//}
	std::shared_ptr<AbstractEventListenerGroup> eventListenerGroup;
	if (eventListenerGroups.get(evt->getType(), eventListenerGroup) && eventListenerGroup != 0) {
		eventListenerGroup->handleEvent(evt);
	}

	//switch (evt->getType())
	//{
	//case DeviceEventType::Refresh:
	//	//const RefreshDeviceEvent& rde = dynamic_cast<const RefreshDeviceEvent&>(evt);
	//	//auto rde = std::dynamic_pointer_cast<RefreshDeviceEvent>(evt);
	//	//if (rde != 0) {
	//	//	refreshListeners.handleEvent(rde);
	//	//}

	//	std::shared_ptr<RefreshDeviceEvent> rde;
	//	if (DeviceEvent::convert<RefreshDeviceEvent>(evt, rde)) {
	//		refreshListeners.handleEvent(rde);
	//	}
	//	break;
	//}


	//try {


	//}
	//catch (const std::bad_cast& e)
	//{
	//	e.what();
	//	//std::cout << "Caught bad cast\n";
	//}

}

