
#include "LocalDevice.h"
#include "LocalDeviceEventDispatcher.h"
#include "DeviceEventReceiver.h"

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


LocalDevice::LocalDevice(const std::string& name, const std::string& address, unsigned short module,
	const std::string& targetServer) : id(name, address, module, targetServer)
{
	std::shared_ptr<DeviceCollectionPolicy> policy = std::make_shared<DeviceCollectionPolicy>();;
	localCollection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>(policy);
//	localCollection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>();

	deviceEventDispatcher = std::make_shared<LocalDeviceEventDispatcher>();

	deviceEventReceiver = std::make_shared<DeviceEventReceiver>(id, localCollection);
}

LocalDevice::~LocalDevice()
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


