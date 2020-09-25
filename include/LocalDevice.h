#ifndef STI_DEVICE_LOCALDEVICE_H
#define STI_DEVICE_LOCALDEVICE_H

#include "Device.h"
#include "DeviceID.h"
#include "DeviceCollection.h"
#include "LocalCollection.h"


#include <string>

namespace STI
{
namespace Device
{

class DeviceEventReceiver;
class LocalDeviceEventDispatcher;

class DeviceCollectionPolicy : public STI::Utils::LocalCollection<DeviceID, Device>::LocalCollectionPolicy
{
	bool include(const STI::Device::DeviceID& key) const { return true; }
	bool replace(const STI::Device::DeviceID& oldKey, const STI::Device::DeviceID& newKey) const { return (oldKey == newKey); }
};


class LocalDevice : public Device
{
public:
	
	LocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~LocalDevice();


	bool refresh() { return true; }

	void write(unsigned input);	//temp

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getEventDispatcher(std::shared_ptr<DeviceEventDispatcher>& dispatcher);
	void getEventReceiver(std::shared_ptr<DeviceEventReceiver>& receiver);

	DeviceID id;


private:

	std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>> localCollection;
	std::shared_ptr<LocalDeviceEventDispatcher> deviceEventDispatcher;
	std::shared_ptr<DeviceEventReceiver> deviceEventReceiver;
};

} //Device
} //STI

#endif
