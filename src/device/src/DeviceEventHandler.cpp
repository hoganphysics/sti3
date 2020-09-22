

#include "DeviceEventHandler.h"
#include "DeviceEvent.h"


using STI::Device::DeviceEventHandler;
using STI::Device::DeviceEvent;


DeviceEventHandler::DeviceEventHandler()
{
	start();
}

DeviceEventHandler::~DeviceEventHandler()
{
}

bool DeviceEventHandler::hasListeners(const std::shared_ptr<DeviceEvent>& evt)
{
	return listenersTypes.count(evt->getType()) == 1;
}

void DeviceEventHandler::handleEvent(const std::shared_ptr<DeviceEvent>& evt)
{
	////hasListeners
	//if (!hasListeners(evt)) {
	//	if (genericListeners.size() > 0) {
	//		genericListeners.handleEvent(evt);
	//	}
	//	return;
	//}

	switch (evt->getType())
	{
	case DeviceEventType::Refresh:
		//const RefreshDeviceEvent& rde = dynamic_cast<const RefreshDeviceEvent&>(evt);
		auto rde = std::dynamic_pointer_cast<RefreshDeviceEvent>(evt);
		if (rde != 0) {
			refreshListeners.handleEvent(rde);
		}
		break;
	}


	//try {


	//}
	//catch (const std::bad_cast& e)
	//{
	//	e.what();
	//	//std::cout << "Caught bad cast\n";
	//}

}

