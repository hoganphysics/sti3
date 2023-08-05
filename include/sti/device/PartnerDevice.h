#ifndef STI_DEVICE_PARTNERDEVICE_H
#define STI_DEVICE_PARTNERDEVICE_H


#include <sti/device/Attribute.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceCollection.h>

#include <sti/fwd/RawEvent_fwd.h>
#include <sti/engine/RawEventTargetChannel.h>

#include <sti/utils/MixedValue.h>

#include <string>
#include <memory>

namespace STI
{
namespace Device
{

class LocalDevice;


class PartnerDevice : public Device 
{
public:

	PartnerDevice(LocalDevice* localDevice, const std::shared_ptr<Device>& partnerDevice);
	virtual ~PartnerDevice() {}
	
	const DeviceID getID() const;
	void kill();
	bool refresh();

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<AttributeManager>& manager);
	bool getPersistenceManager(std::shared_ptr<PersistenceManager>& manager);

	void addEvent(const STI::Engine::RawEvent& evt, const STI::Engine::RawEvent& referenceEvent);
	void addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, const STI::Utils::MixedValue& value, const STI::Engine::RawEvent& referenceEvent);
	void addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, const STI::Utils::MixedValue& value, const STI::Engine::RawEventType& eventType, const STI::Engine::RawEvent& referenceEvent);

	bool write(short channel, const STI::Utils::MixedValue& value);
	bool read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
	void stopRW();

	std::string getAttributeValue(const std::string& key);
	bool setAttributeValue(const std::string& key, const std::string& value);
	bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute);

private:
	
	void activate() {}
	void disable() {}
	void attachMessageListenerForwarder(const std::shared_ptr<DeviceMessageListenerForwarder>& forwarder) {}

	LocalDevice* localDevice;
	std::shared_ptr<Device> device;
};


} //Device
} //STI

#endif
