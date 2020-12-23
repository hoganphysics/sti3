#ifndef STI_NETWORK_REMOTEDEVICE_H
#define STI_NETWORK_REMOTEDEVICE_H

#include "deviceNet.h"

#include "Device.h"
#include "DeviceCollection.h"
#include "TDeviceRefInterface.h"
#include "DeviceEventDispatcher.h"

#include <memory>

namespace STI
{
namespace Network
{

class RemoteDevice : public STI::Device::Device, 
					 public STI::Network::TDeviceRefInterface	//mixin
{
public:

	RemoteDevice(::STI::TNetwork::TDevice_ptr device);

	bool refresh();

	STI::Device::DeviceID getID();

	void write(unsigned input);
	
	void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) {}

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getEventDispatcher(std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);

private:

	bool getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice);

	::STI::TNetwork::TDevice_var _tDevice;		//remote reference

};


} //Network
} //STI


#endif

