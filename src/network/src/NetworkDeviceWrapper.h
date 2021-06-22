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
#include "NetworkFileHolder.h"

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

		std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;
		getMessageDispatcher(dispatcher);
		std::shared_ptr<STI::Device::ChannelManager> channels;
		getChannelManager(channels);
		std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;
		getCollection(deviceCollection);

		auto networkEngineFactory = std::make_shared<STI::Network::NetworkEventEngineFactory>(getID(), channels, dispatcher, deviceCollection);
		//localDevice->setEngineFactory(networkEngineFactory);

		std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
		localDevice->getEngineScheduler(scheduler);
		scheduler->setEngineFactory(networkEngineFactory);

		auto networkFileHolderFactory = std::make_shared<STI::Network::NetworkFileHolderFactory>();

		localDevice->setFileHolderFactory(networkFileHolderFactory);
	}

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

	void disable()
	{
		if (localDevice != 0) {
			localDevice->disable();
		}
	}

private:

	void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
	{
		if (localDevice != 0) {
			localDevice->setFileHolderFactory(factory);
		}
	}

	void attachMessageListenerForwarder(const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder)
	{
		if (localDevice != 0) {
			localDevice->attachMessageListenerForwarder(forwarder);
		}
	}

	// bool getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice)
	// {
	// 	tDevice = deviceServant._this();
		
	// 	return !CORBA::is_nil(tDevice);
	// }

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

