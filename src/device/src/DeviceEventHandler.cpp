

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

bool DeviceEventHandler::hasListeners(const DeviceEvent& evt)
{
	return listenersTypes.count(evt.getType()) == 1;
}

void DeviceEventHandler::handleEvent(const DeviceEvent& evt)
{
	////hasListeners
	//if (!hasListeners(evt)) {
	//	if (genericListeners.size() > 0) {
	//		genericListeners.handleEvent(evt);
	//	}
	//	return;
	//}

	try {

		switch (evt.getType())
		{
		case DeviceEventType::Refresh:
			const RefreshDeviceEvent& rde = dynamic_cast<const RefreshDeviceEvent&>(evt);
			refreshListeners.handleEvent(rde);
			break;
		}
	}
	catch (const std::bad_cast& e)
	{
		e.what();
		//std::cout << "Caught bad cast\n";
	}

}

