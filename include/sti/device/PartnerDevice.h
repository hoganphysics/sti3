#ifndef STI_DEVICE_PARTNERDEVICE_H
#define STI_DEVICE_PARTNERDEVICE_H


#include <sti/device/Attribute.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceCollection.h>

#include <sti/fwd/RawEvent_fwd.h>
#include <sti/engine/RawEventTargetChannel.h>

#include <sti/utils/MixedValue.h>
#include <sti/utils/MetaData.h>

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
	PartnerDevice(LocalDevice* localDevice, const DeviceID& partnerID, const std::shared_ptr<Device>& partnerDevice);
	virtual ~PartnerDevice() {}
	
	const DeviceID getID() const;
	void kill();
	bool refresh();

	const STI::Utils::MixedValue& getMetaData() const override;
	STI::Utils::MixedValue getMetaData(const std::string& key) const override;

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<AttributeManager>& manager);
	bool getMonitorManager(std::shared_ptr<MonitorManager>& manager);
	bool getPersistenceManager(std::shared_ptr<PersistenceManager>& manager);
	bool getProfileManager(std::shared_ptr<ProfileManager>& manager);
	bool getTaskManager(std::shared_ptr<TaskManager>& manager);
	bool getLogManager(std::shared_ptr<LogManager>& manager);
	bool getVersionManager(std::shared_ptr<VersionManager>& manager) override;
	bool getPostProcessingManager(std::shared_ptr<PostProcessingManager>& manager) override;

	void addEvent(const STI::Engine::RawEvent& evt, const STI::Engine::RawEvent& referenceEvent);
	void addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, const STI::Utils::MixedValue& value, const STI::Engine::RawEvent& referenceEvent);
	void addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, const STI::Utils::MixedValue& value, const STI::Engine::RawEventType& eventType, const STI::Engine::RawEvent& referenceEvent);

	bool write(short channel, const STI::Utils::MixedValue& value);
	bool read(short channel, STI::Utils::MixedValue& data);
	bool read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
	void stopRW();

	std::string getAttribute(const std::string& key);
	bool setAttribute(const std::string& key, const std::string& value);
	bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute);

private:
	
	void activate() {}
	void disable() {}
	void attachMessageListenerForwarder(const std::shared_ptr<DeviceMessageListenerForwarder>& forwarder) {}

	LocalDevice* localDevice;
	DeviceID partnerID;
	std::shared_ptr<Device> device;
	STI::Utils::MetaData metaData;
};


} //Device
} //STI

#endif
