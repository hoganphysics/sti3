#ifndef STI_NETWORK_NETWORKDEVICEWRAPPER_H
#define STI_NETWORK_NETWORKDEVICEWRAPPER_H

#include <sti/device/Device.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include "TDevice_i.h"
#include "TDeviceRefInterface.h"
#include "NetworkEventEngineFactory.h"
#include "orbTypes.h"
#include <sti/engine/EventEngineScheduler.h>
#include "DeviceMessageListenerForwarder.h"
#include "NetworkFileHolder.h"
#include <sti/device/PersistenceManager.h>
// #include "NetworkShotRepositoryWrapper.h"

#include <memory>

namespace STI
{
namespace Network
{

//Thin wrapper around a LocalDevice that also holds a TDevice_i servant of the same Device
class NetworkDevice : public STI::Device::Device,
							 public STI::Network::TDeviceRefInterface	//mixin
{
public:

	NetworkDevice(const std::shared_ptr<STI::Device::Device>& device);
	virtual ~NetworkDevice() {}

	void getCollection(std::shared_ptr<STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device>>& collection)
	{
		if (localDevice != 0) {
			localDevice->getCollection(collection);			
		}
	}

	void getMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher)
	{
		if (localDevice != 0) {
			localDevice->getMessageDispatcher(dispatcher);
		}
	}

	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
	{
		return localDevice != 0 && localDevice->getEngineScheduler(scheduler);
	}

	void getChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager)
	{
		if (localDevice != 0) {
			localDevice->getChannelManager(manager);
		}
	}

	void getAttributeManager(std::shared_ptr<STI::Device::AttributeManager>& manager)
	{
		if (localDevice != 0) {
			localDevice->getAttributeManager(manager);
		}
	}

	bool getPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& manager)
	{
		return localDevice != 0 && localDevice->getPersistenceManager(manager);
	}

	const STI::Device::DeviceID getID() const 
	{
		if (localDevice != 0) {
			return localDevice->getID();
		}

		STI::Device::DeviceID missing;
		return missing;
	}

	bool refresh() { return localDevice != 0 && localDevice->refresh(); }

	void kill() 
	{
		if (localDevice != 0) {
			localDevice->kill();
		}
	}

	void activate()
	{
		if (localDevice != 0) {
			localDevice->activate();
		}
	}

	void disable()
	{
		if (localDevice != 0) {
			localDevice->disable();
		}
	}

private:

	void attachMessageListenerForwarder(const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder)
	{
		if (localDevice != 0) {
			localDevice->attachMessageListenerForwarder(forwarder);
		}
	}

	bool getTDeviceRef(STI::TNetwork::TDevice_var& tDevice)
	{
		tDevice = deviceServant._this();
		
		return !CORBA::is_nil(tDevice);
	}

	std::shared_ptr<STI::Device::Device> localDevice;
	STI::TNetwork::TDevice_i deviceServant;
};


} //Network
} //STI


#endif

