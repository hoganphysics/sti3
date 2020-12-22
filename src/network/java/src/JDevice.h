#ifndef STI_DEVICE_JDEVICE_H
#define STI_DEVICE_JDEVICE_H

#include "Device.h"
#include "DeviceCollection.h"

#include "JEventEngineScheduler.h"

#include <memory>
#include <string>

namespace STI
{

// namespace Engine
// {

// class JEventEngineScheduler;

// } //Engine

namespace Device
{

class JDeviceCollection;
class JDeviceEventReceiver;
class JDeviceEventDispatcher;

//Java Device wrapper
class JDevice : public STI::Device::Device
{
public:
	
	JDevice(const std::shared_ptr<STI::Device::Device>& device);
	JDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~JDevice();

	DeviceID getID();

	//Device
	bool refresh();
	virtual void write(unsigned input);	//temp

	std::shared_ptr<STI::Device::JDeviceCollection> getCollection();
	std::shared_ptr<STI::Device::JDeviceEventDispatcher> getEventDispatcher();
	std::shared_ptr<STI::Device::JEventEngineScheduler> getEngineScheduler();

private:
	friend class JLocalDevice;

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getEventDispatcher(std::shared_ptr<DeviceEventDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);

    std::shared_ptr<Device> wrappedDevice;

};

} //Device
} //STI

#endif
