#ifndef STI_NETWORK_NETWORKDEVICEWRAPPER_H
#define STI_NETWORK_NETWORKDEVICEWRAPPER_H

#include "Device.h"
#include "DeviceMessageDispatcher.h"
#include "TDevice_i.h"
#include "TDeviceRefInterface.h"
#include "NetworkEventEngineFactory.h"
#include "orbTypes.h"
#include "EventEngineScheduler.h"
#include "DeviceMessageListenerForwarder.h"

#include <memory>

namespace STI
{
namespace Network
{

//Thin wrapper around a LocalDevice that also holds a TDevice_i servant of the same Device
class NetworkDeviceWrapper : public STI::Device::Device,
							 public STI::Network::TDeviceRefInterface	//mixin
{
public:

	NetworkDeviceWrapper(const std::shared_ptr<STI::Device::Device>& device)
		: localDevice(device), deviceServant(device) 
	{
		auto networkEngineFactory = std::make_shared<STI::Engine::NetworkEventEngineFactory>();
		//localDevice->setEngineFactory(networkEngineFactory);

		std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
		localDevice->getEngineScheduler(scheduler);
		scheduler->setEngineFactory(networkEngineFactory);
	}

	void getCollection(std::shared_ptr<STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device>>& collection)
	{
		localDevice->getCollection(collection);
	}

	void getMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher)
	{
		localDevice->getMessageDispatcher(dispatcher);
	}

	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
	{
		return localDevice->getEngineScheduler(scheduler);
	}

	void getChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager)
	{
		localDevice->getChannelManager(manager);
	}

	void getAttributeManager(std::shared_ptr<STI::Device::AttributeManager>& manager)
	{
		localDevice->getAttributeManager(manager);
	}

	const STI::Device::DeviceID getID() const { return localDevice->getID(); }

	bool refresh() { return localDevice->refresh(); }

private:

	void attachMessageListenerForwarder(const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder)
	{
		localDevice->attachMessageListenerForwarder(forwarder);
	}

	bool getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice)
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

