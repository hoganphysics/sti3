#ifndef STI_DEVICE_LOCALDEVICE_H
#define STI_DEVICE_LOCALDEVICE_H

#include "Device.h"
#include "DeviceID.h"
#include "DeviceCollection.h"
#include "DeviceMessageListener.h"
#include "LocalCollection.h"
#include "fwd/EventEngineScheduler_fwd.h"
#include "DeviceEventParser.h"
#include "EngineID.h"
#include "fwd/Channel_fwd.h"
#include "fwd/ChannelManager_fwd.h"
#include "MixedValue.h"
#include <string>
#include <set>


namespace STI
{

namespace Device
{

class DeviceMessageReceiver;
class LocalDeviceMessageDispatcher;
class LocalChannelManager;
class LocalChannel;
class LocalDevice;
class LocalAttribute;
class LocalAttributeManager;
class DeviceMessageListenerID;


class DeviceCollectionPolicy : public STI::Utils::LocalCollection<DeviceID, Device>::LocalCollectionPolicy
{
public:
	DeviceCollectionPolicy(LocalDevice* device) : device(device) {}
	
	bool include(const STI::Device::DeviceID& key) const;
	bool replace(const STI::Device::DeviceID& oldKey, const STI::Device::DeviceID& newKey) const { return (oldKey == newKey); }

private:
	LocalDevice* device;
};


class LocalDevice : public Device, public STI::Engine::DeviceEventParser
{
public:
	
	LocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~LocalDevice();

	const DeviceID getID() const;

	bool refresh() { return true; }
//	void write(unsigned input);	//temp

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
	void getMessageReceiver(std::shared_ptr<DeviceMessageReceiver>& receiver);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<AttributeManager>& manager);

	bool getEngineScheduler(std::shared_ptr<STI::Engine::LocalEventEngineScheduler>& scheduler);	//temp

	void addEventEngine(const STI::Engine::EngineID& engineID);

	LocalChannel& addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName);

	LocalAttribute& addAttribute(const std::string& key, const std::string& initialValue);
	LocalAttribute& addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues);

	void addPartner(const DeviceID& id) { partnerDevices.insert(id); }


	virtual void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) {}
	void getEventTargets(std::set<DeviceID>& targetIDs)
	{
		targetIDs = eventTargets;
	}

	bool write(short channel, const STI::Utils::MixedValue& value);
	bool read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);

private:

	virtual bool writeChannel(short channel, const STI::Utils::MixedValue& value) { return false; }
	virtual bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) { return false; }


	void addEventTarget(const STI::Device::DeviceID& id);
	virtual bool isEventTarget(const DeviceID& id);

	class DeviceCollectionListener : public STI::Utils::LocalCollectionListenerAdapter<DeviceID>
	{
	public:
		DeviceCollectionListener(LocalDevice* localDevice) : localDevice(localDevice) {}

		void add(const DeviceID& id);
		void remove(const DeviceID& id);

		LocalDevice* localDevice;
	};
		
	DeviceMessageListenerID schedulerMessageLID;

	friend DeviceCollectionPolicy;
	bool isPartnerDevice(const DeviceID& id);
	bool isTargetServerOf(const DeviceID& id);

	DeviceID id;
	std::set<DeviceID> eventTargets;	//this LocalDevice can generate events for these (partner) devices

	std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>> localCollection;
	std::shared_ptr<LocalDeviceMessageDispatcher> deviceMessageDispatcher;
	std::shared_ptr<DeviceMessageReceiver> deviceMessageReceiver;
	std::shared_ptr<STI::Engine::LocalEventEngineScheduler> eventEngineScheduler;
	std::shared_ptr<LocalChannelManager> localChannelManager;
	std::shared_ptr<LocalAttributeManager> localAttributeManager;

	std::set<DeviceID> partnerDevices;

};

} //Device
} //STI

#endif
