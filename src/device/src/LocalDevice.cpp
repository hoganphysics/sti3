
#include "LocalDevice.h"
#include "LocalDeviceEventDispatcher.h"
#include "DeviceEventReceiver.h"
#include "LocalEventEngineScheduler.h"
#include "DeviceEventListener.h"
#include "DeviceEvent.h"

#include <memory>
#include <iostream>

using std::cout;
using std::endl;

using STI::Device::Device;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Device::DeviceEventDispatcher;
using STI::Device::LocalDeviceEventDispatcher;
using STI::Device::DeviceEventReceiver;
using STI::Engine::LocalEventEngineScheduler;

LocalDevice::LocalDevice(const std::string& name, const std::string& address, unsigned short module,
	const std::string& targetServer) : id(name, address, module, targetServer)
{
	std::shared_ptr<DeviceCollectionPolicy> policy = std::make_shared<DeviceCollectionPolicy>();;
	localCollection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>(policy);
//	localCollection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>();

	deviceEventDispatcher = std::make_shared<LocalDeviceEventDispatcher>();

	deviceEventReceiver = std::make_shared<DeviceEventReceiver>(id, localCollection);

	auto deviceCollectionListener = std::make_shared<STI::Device::LocalDevice::DeviceCollectionListener>(this);
	localCollection->addListener(deviceCollectionListener);

	eventEngineScheduler = std::make_shared<LocalEventEngineScheduler>(this);

}

LocalDevice::~LocalDevice()
{

}



//LocalDeviceCollection event handler
void LocalDevice::DeviceCollectionListener::add(const DeviceID& id)
{
	// Listen to EngineScheduler messages from:
	// 1) Declared event targets and 2) any device that has this device as a target server.

	auto it = localDevice->eventTargets.find(id);
	bool isEventTarget = (it != localDevice->eventTargets.end());

	if( isEventTarget || id.getTargetServerID() == localDevice->getID().getID() ) {
		
		//std::shared_ptr<STI::Device::DeviceEventListener<STI::Device::EngineSchedulerMessage>> listener = //localDevice->eventEngineScheduler;
		auto listener = std::static_pointer_cast<STI::Device::DeviceEventListener<STI::Device::EngineSchedulerMessage>>(localDevice->eventEngineScheduler);
		//localDevice->deviceEventReceiver->addListener(sourceDevice, listenerID, listenerX);
		STI::Device::DeviceEventListenerID listenerID;
		listenerID.name = localDevice->getID().getID() + "::EventEngineScheduler";
		listenerID.type = STI::Device::DeviceEventType::EngineScheduler;

		localDevice->deviceEventReceiver->addListener(id, listenerID, listener);	//listen to events on new device 'id'
	}
}

//LocalDeviceCollection event handler
void LocalDevice::DeviceCollectionListener::remove(const DeviceID& id)
{

}

DeviceID LocalDevice::getID()
{
	return id;
}

void LocalDevice::write(unsigned input)
{
	cout << "writting: " << input << endl;
}


void LocalDevice::getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
{
	collection = localCollection;
}

void LocalDevice::getEventDispatcher(std::shared_ptr<DeviceEventDispatcher>& dispatcher)
{
	dispatcher = deviceEventDispatcher;
}

void LocalDevice::getEventReceiver(std::shared_ptr<DeviceEventReceiver>& receiver)
{
	receiver = deviceEventReceiver;
}

bool LocalDevice::getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
{
	scheduler = eventEngineScheduler;
	return scheduler != 0;
}

bool LocalDevice::getEngineScheduler(std::shared_ptr<STI::Engine::LocalEventEngineScheduler>& scheduler)
{
	scheduler = eventEngineScheduler;
	return scheduler != 0;
}

