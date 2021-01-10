#ifndef STI_NETWORK_REMOTEDEVICE_H
#define STI_NETWORK_REMOTEDEVICE_H

#include "deviceNet.h"

#include "Device.h"
#include "DeviceCollection.h"
#include "TDeviceRefInterface.h"
#include "DeviceMessageDispatcher.h"
#include "fwd/ChannelManager_fwd.h"

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

	const STI::Device::DeviceID getID() const;

	void write(unsigned input);
	
	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager);

private:

	bool getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice);

	::STI::TNetwork::TDevice_var _tDevice;		//remote reference

};


} //Network
} //STI


#endif

