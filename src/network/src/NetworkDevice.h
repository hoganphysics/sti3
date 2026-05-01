#ifndef STI_NETWORK_NETWORKDEVICEWRAPPER_H
#define STI_NETWORK_NETWORKDEVICEWRAPPER_H

#include <sti/device/Device.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/PersistenceManager.h>
#include <sti/engine/EventEngineScheduler.h>

#include "DeviceMessageListenerForwarder.h"
#include "NetworkEventEngineFactory.h"
#include "NetworkFileHolder.h"
#include "TDevice_i.h"
#include "TDeviceRefInterface.h"

#include "generated/orbTypes.h"
#include "ServantHolder.h"

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

	bool getMonitorManager(std::shared_ptr<STI::Device::MonitorManager>& manager)
	{
		return localDevice != 0 && localDevice->getMonitorManager(manager);
	}

	bool getPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& manager)
	{
		return localDevice != 0 && localDevice->getPersistenceManager(manager);
	}

	bool getProfileManager(std::shared_ptr<STI::Device::ProfileManager>& manager)
	{
		return localDevice != 0 && localDevice->getProfileManager(manager);
	}

	bool getTaskManager(std::shared_ptr<STI::Device::TaskManager>& manager)
	{
		return localDevice != 0 && localDevice->getTaskManager(manager);
	}
	
	bool getLogManager(std::shared_ptr<STI::Device::LogManager>& manager)
	{
		return localDevice != 0 && localDevice->getLogManager(manager);
	}

	bool getVersionManager(std::shared_ptr<STI::Device::VersionManager>& manager) override
	{
		return localDevice != 0 && localDevice->getVersionManager(manager);
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

	bool write(short channel, const STI::Utils::MixedValue& value)
	{
		if (localDevice != 0) {
			return localDevice->write(channel, value);
		}
		return false;
	}

	bool read(short channel, STI::Utils::MixedValue& data)
	{
		return read(channel, STI::Utils::MixedValueType::Empty, data);
	}

	bool read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
	{
		if (localDevice != 0) {
			return localDevice->read(channel, value, data);
		}
		return false;
	}

	void stopRW()
	{
		if (localDevice != 0) {
			localDevice->stopRW();
		}
	}

	std::string getAttribute(const std::string& key)
	{
		if (localDevice != 0) {
			return localDevice->getAttribute(key);
		}
		return "";
	}

	bool setAttribute(const std::string& key, const std::string& value)
	{
		if (localDevice != 0) {
			return localDevice->setAttribute(key, value);
		}
		return false;
	}

	bool getAttribute(const std::string& key, std::shared_ptr<STI::Device::Attribute>& attribute)
	{
		if (localDevice != 0) {
			return localDevice->getAttribute(key, attribute);
		}
		return false;
	}

	bool addto(const STI::Network::HubID& target) 
	{ 
		if (localDevice != 0) {
			return localDevice->addto(target);
		}
		return true; 
	}
	void setRemoveCB(const std::function<void(void)>& remover) override 
	{
		if (localDevice != 0) {
			localDevice->setRemoveCB(remover);
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
		tDevice = deviceServantHolder.getRefVar();
		
		return !CORBA::is_nil(tDevice);
	}

	std::shared_ptr<STI::Device::Device> localDevice;
	// STI::TNetwork::TDevice_i deviceServant;
	ServantHolder<STI::TNetwork::TDevice_i, STI::TNetwork::TDevice> deviceServantHolder;
};


} //Network
} //STI


#endif
