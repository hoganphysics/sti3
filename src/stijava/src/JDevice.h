#ifndef STI_DEVICE_JDEVICE_H
#define STI_DEVICE_JDEVICE_H

#include <sti/device/Device.h>
#include <sti/device/DeviceCollection.h>
#include "JEventEngineScheduler.h"
#include <sti/utils/FileHolderFactory.h>

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
class JProfileManager;
class JTaskManager;
class JLogManager;


//Java Device wrapper
class JDevice : public STI::Device::Device
{
public:
	
	JDevice(const std::shared_ptr<STI::Device::Device>& device);
	JDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~JDevice();

	const DeviceID getID() const;
	void kill();
	void activate();
	void disable();

	//Device
	bool refresh();
//	virtual void write(unsigned input);	//temp

	std::string getAttribute(const std::string& key);
	bool setAttribute(const std::string& key, const std::string& value);

	std::shared_ptr<STI::Device::JDeviceCollection> getCollection();
	std::shared_ptr<STI::Device::JDeviceMessageDispatcher> getMessageDispatcher();
	std::shared_ptr<STI::Engine::JEventEngineScheduler> getEngineScheduler();
	std::shared_ptr<STI::Device::JChannelManager> getChannelManager();
	std::shared_ptr<STI::Device::JAttributeManager> getAttributeManager();
	std::shared_ptr<STI::Device::JPersistenceManager> getPersistenceManager();

	std::shared_ptr<STI::Device::JProfileManager> getProfileManager();
	std::shared_ptr<STI::Device::JTaskManager> getTaskManager();
	std::shared_ptr<STI::Device::JLogManager> getLogManager();

private:

	friend class JLocalDevice;

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<AttributeManager>& manager);
	bool getPersistenceManager(std::shared_ptr<PersistenceManager>& manager);
	bool getProfileManager(std::shared_ptr<STI::Device::ProfileManager>& manager);
	bool getTaskManager(std::shared_ptr<STI::Device::TaskManager>& manager);
	bool getLogManager(std::shared_ptr<STI::Device::LogManager>& manager);

	bool write(short channel, const STI::Utils::MixedValue& value);
	bool read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
	void stopRW();

	bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute);

	void attachMessageListenerForwarder(const std::shared_ptr<DeviceMessageListenerForwarder>& forwarder) {}
	void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory) {}

    std::shared_ptr<Device> wrappedDevice;

};

} //Device
} //STI

#endif
