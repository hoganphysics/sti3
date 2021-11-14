#ifndef STI_DEVICE_JDEVICE_H
#define STI_DEVICE_JDEVICE_H

#include "Device.h"
#include "DeviceCollection.h"
#include "JEventEngineScheduler.h"
#include "FileHolderFactory.h"

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
class JDeviceMessageReceiver;
class JDeviceMessageDispatcher;
class JChannelManager;
class JAttributeManager;
class JPersistenceManager;


//Java Device wrapper
class JDevice : public STI::Device::Device
{
public:
	
	JDevice(const std::shared_ptr<STI::Device::Device>& device);
	JDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~JDevice();

	const DeviceID getID() const;
	void kill() {}
	void disable() {}

	//Device
	bool refresh();
//	virtual void write(unsigned input);	//temp

	std::shared_ptr<STI::Device::JDeviceCollection> getCollection();
	std::shared_ptr<STI::Device::JDeviceMessageDispatcher> getMessageDispatcher();
	std::shared_ptr<STI::Engine::JEventEngineScheduler> getEngineScheduler();
	std::shared_ptr<STI::Device::JChannelManager> getChannelManager();
	std::shared_ptr<STI::Device::JAttributeManager> getAttributeManager();
	std::shared_ptr<STI::Device::JPersistenceManager> getPersistenceManager();

private:

	friend class JLocalDevice;

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<AttributeManager>& manager);
	void getPersistenceManager(std::shared_ptr<PersistenceManager>& manager);

	void attachMessageListenerForwarder(const std::shared_ptr<DeviceMessageListenerForwarder>& forwarder) {}
	void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory) {}

    std::shared_ptr<Device> wrappedDevice;

};

} //Device
} //STI

#endif
