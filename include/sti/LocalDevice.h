#ifndef STI_DEVICE_LOCALDEVICE_H
#define STI_DEVICE_LOCALDEVICE_H


#include <sti/fwd/Channel_fwd.h>
#include <sti/fwd/EventEngineScheduler_fwd.h>

#include <sti/device/AttributeManager.h>
#include <sti/device/ChannelManager.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceCollection.h>
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/LocalAttribute.h>
#include <sti/device/LocalChannel.h>
#include <sti/device/ServerMessageRelayer.h>

#include <sti/engine/DeviceEventParser.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/ParseTicketManager.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ResultTicketManager.h>
#include <sti/engine/SynchronousEvent.h>

#include <sti/utils/Configuration.h>
#include <sti/utils/LocalCollection.h>
#include <sti/utils/MixedValue.h>

#include <map>
#include <mutex>
#include <set>
#include <string>


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
class LocalPersistenceManager;
class PersistenceManager;
class DeviceCollectionPolicy;


class LocalDevice : public Device, public STI::Engine::DeviceEventParser
{
public:
	
	LocalDevice(const std::map<std::string, std::string>& config);
	LocalDevice(const STI::Utils::Configuration& config, const std::string& section="");
	LocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~LocalDevice();

	const DeviceID getID() const;

	bool refresh() { return true; }
	void kill() {}
	void disable();

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageReceiver(std::shared_ptr<DeviceMessageReceiver>& receiver);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<AttributeManager>& manager);
	bool getPersistenceManager(std::shared_ptr<PersistenceManager>& manager);

	bool getEngineScheduler(std::shared_ptr<STI::Engine::LocalEventEngineScheduler>& scheduler);	//temp

	void addEventEngine(const STI::Engine::EngineID& engineID);

	LocalChannel& addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName);
	void addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName, std::shared_ptr<STI::Device::LocalChannel>& channel);


	void addAttribute(const std::string& key, const std::string& initialValue, std::shared_ptr<STI::Device::LocalAttribute>& attribute);
	void addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues, std::shared_ptr<STI::Device::LocalAttribute>& attribute);

	LocalAttribute& addAttribute(const std::string& key, const std::string& initialValue);
	LocalAttribute& addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues);

	template<typename T>
	LocalAttribute& addAttribute(const std::string& key, const T& initialValue)
	{
		return addAttribute(key, STI::Utils::valueToString(initialValue));
	}
	template<typename T>
	LocalAttribute& addAttribute(const std::string& key, const T& initialValue, std::vector<std::string> allowedValues)
	{
		return addAttribute(key, STI::Utils::valueToString(initialValue), allowedValues);
	}

	void addPartner(const DeviceID& id);
	void addEventTarget(const DeviceID& id);

	void sendMessage(const std::shared_ptr<DeviceMessage>& mess);

	virtual void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) {}
	void getEventTargets(std::set<DeviceID>& targetIDs);

	bool write(short channel, const STI::Utils::MixedValue& value);
	bool read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
	void stopRW();

	bool writeChannelDefault(short channel, const STI::Utils::MixedValue& value);
	bool readChannelDefault(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
	bool playSingleEvent(const STI::Engine::RawEvent& event, std::shared_ptr<STI::Engine::ResultTicket>& resultTicket);

	virtual bool isEventTarget(const DeviceID& id);

	void addCollectionListener(const std::shared_ptr<STI::Utils::LocalCollectionListenerAdapter<DeviceID>>& listener);

	std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& filename);

private:

	void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher);


	virtual bool writeChannel(short channel, const STI::Utils::MixedValue& value) { return writeChannelDefault(channel, value); }
	virtual bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) { return readChannelDefault(channel, value, data); }


	friend class DeviceMessageListenerForwarder;
	void attachMessageListenerForwarder(const std::shared_ptr<DeviceMessageListenerForwarder>& forwarder) {}	//not needed for local device
	// DeviceMessageListenerForwarder listenerForwarder;
	std::shared_ptr<DeviceMessageListenerForwarder> listenerForwarder;


	class DeviceCollectionListener : public STI::Utils::LocalCollectionListenerAdapter<DeviceID>
	{
	public:
		DeviceCollectionListener(LocalDevice* localDevice) : localDevice(localDevice) {}

		void add(const DeviceID& id);
		void remove(const DeviceID& id);

		LocalDevice* localDevice;
		//MessageGrouper<CollectionMessage> messageGrouper;
	};
	
	//DeviceMessageListenerID collectionMessageLID;
	DeviceMessageListenerID schedulerMessageLID;

	//std::vector<DeviceMessageListenerID> messageListenerIDs;
	
	std::shared_ptr<STI::Engine::ParseTicketManager<>> parseTicketManager;
	std::shared_ptr<STI::Engine::ResultTicketManager<>> resultTicketManager;

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
	std::shared_ptr<LocalPersistenceManager> localPersistenceManager;
	
	std::shared_ptr<ServerMessageRelayer> serverMessageRelayer;

	//std::shared_ptr<STI::Engine::SerializedRepository> localSerializedRepository;

	std::set<DeviceID> partnerDevices;

	mutable std::mutex deviceMutex;

};


class DeviceCollectionPolicy : public STI::Utils::LocalCollection<DeviceID, Device>::LocalCollectionPolicy
{
public:
	DeviceCollectionPolicy(LocalDevice* device) : device(device) {}
	
	bool include(const STI::Device::DeviceID& key) const;
	bool replace(const STI::Device::DeviceID& oldKey, const STI::Device::DeviceID& newKey) const { return (oldKey == newKey); }

private:
	LocalDevice* device;
};

} //Device
} //STI

#endif
