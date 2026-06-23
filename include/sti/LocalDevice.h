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
#include <sti/device/PartnerDevice.h>
#include <sti/device/PostProcessingManager.h>
#include <sti/device/ProfileManager.h>
#include <sti/device/ServerMessageRelayer.h>
#include <sti/device/TaskManager.h>
#include <sti/device/LogManager.h>
#include <sti/device/Logger.h>

#include <sti/engine/DeviceEventParser.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineConflictPolicy.h>
#include <sti/engine/EventConflictException.h>
#include <sti/engine/EventParsingException.h>
#include <sti/engine/EngineTriggerTarget.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/ParseTicketManager.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ResultTicketManager.h>
#include <sti/engine/ShotRepository.h>
#include <sti/engine/SynchronousEvent.h>

#include <sti/utils/Configuration.h>
#include <sti/utils/LocalCollection.h>
#include <sti/utils/MetaData.h>
#include <sti/utils/MixedValue.h>

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <functional>


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
class LocalMonitor;
class MonitorManager;
class LocalMonitorManager;
class AutoMonitor;
class DeviceMessageListenerID;
class LocalPersistenceManager;
class LocalProfileManager;
class PersistenceManager;
class DeviceCollectionPolicy;
class LocalTaskManager;
class LocalLogManager;
class VersionInfo;
class VersionManager;
class LocalPostProcessingManager;


class LocalDevice : public Device, public STI::Engine::DeviceEventParser, public STI::Engine::EngineTriggerTarget
{
public:
	
	LocalDevice(const std::map<std::string, std::string>& config);
	LocalDevice(const STI::Utils::Configuration& config, const std::string& section="");
	LocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer, 
		const STI::Utils::Configuration& config=STI::Utils::Configuration());
	virtual ~LocalDevice();

	const DeviceID getID() const;

	const STI::Utils::MixedValue& getMetaData() const override;
	STI::Utils::MixedValue getMetaData(const std::string& key) const override;
	LocalDevice& addMetaData(const std::string& key, const STI::Utils::MixedValue& value);
	LocalDevice& addMetaDataList(const std::string& key, const std::vector<std::string>& values);
	LocalDevice& setColor(const std::string& color);
	LocalDevice& setDescription(const std::string& description);
	LocalDevice& setHelp(const std::string& help);

	bool refresh() override;
	void kill();
	void activate();
	void disable();

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	bool getMessageReceiver(std::shared_ptr<DeviceMessageReceiver>& receiver);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<AttributeManager>& manager);
	bool getPersistenceManager(std::shared_ptr<PersistenceManager>& manager);
	bool getProfileManager(std::shared_ptr<ProfileManager>& manager);
	bool getTaskManager(std::shared_ptr<TaskManager>& manager);
	bool getMonitorManager(std::shared_ptr<MonitorManager>& manager);
	bool getLogManager(std::shared_ptr<LogManager>& manager);
	bool getVersionManager(std::shared_ptr<VersionManager>& manager) override;
	bool getPostProcessingManager(std::shared_ptr<PostProcessingManager>& manager) override;

	bool getFileServer(std::shared_ptr<STI::Utils::FileServer>& fileServer);

	bool getEngineScheduler(std::shared_ptr<STI::Engine::LocalEventEngineScheduler>& scheduler);
	bool getMonitorManager(std::shared_ptr<LocalMonitorManager>& manager);
	bool getLogManager(std::shared_ptr<LocalLogManager>& manager);

	void addEventEngine(const STI::Engine::EngineID& engineID);

	LocalChannel& addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName);
	void addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName, std::shared_ptr<STI::Device::LocalChannel>& channel);

	LocalChannel& addInputChannel(unsigned short channelNumber, STI::Utils::MixedValueType inputType, const std::string& defaultName);
	LocalChannel& addInputChannel(unsigned short channelNumber, STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName);
	LocalChannel& addOutputChannel(unsigned short channelNumber, STI::Utils::MixedValueType outputType, const std::string& defaultName);

	void addAttribute(const std::string& key, const std::string& initialValue, std::shared_ptr<STI::Device::LocalAttribute>& attribute);
	void addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues, std::shared_ptr<STI::Device::LocalAttribute>& attribute);

	LocalAttribute& addAttribute(const std::string& key, const std::string& initialValue);
	LocalAttribute& addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues);

    LocalMonitor& addMonitor(const std::string& id);

    AutoMonitor& addAutoMonitor(const std::string& id, double updateInterval_s,
        const std::function<STI::Utils::MixedValue(void)>& updater);
    void addAutoMonitor(const std::string& id, double updateInterval_s,
        const std::function<STI::Utils::MixedValue(void)>& updater,
        std::shared_ptr<STI::Device::AutoMonitor>& monitor);

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
	void addPartner(const DeviceID& id, const std::string& alias);
	void addEventTarget(const DeviceID& id);
	void addEventTarget(const DeviceID& id, const std::string& alias);

	PartnerDevice partner(const DeviceID& id);
	PartnerDevice partner(const std::string& alias);

	Logger& log();
    Logger& log(const std::string& name);

	void addTask(const std::shared_ptr<STI::Utils::Task>& task);
	void addPostProcessingTarget(const std::string& name, STI::Device::PostProcessingFunction function,
		const std::string& description = "");
	bool addVersionInfo(const VersionInfo& version);
	bool addVersionInfo(const std::string& component, const std::string& version);

	void sendMessage(const std::shared_ptr<DeviceMessage>& mess);

	virtual void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) { parseEventsDefault(events, synchedEvents); }
	void getEventTargets(std::set<DeviceID>& targetIDs);
	virtual double getMinimumEventSpacing() { return 1000.0; }	//in nanoseconds
	virtual double getMinimumEventStartTime() { return 1000.0; }	//in nanoseconds
	
	virtual void requestTrigger(const STI::Engine::EngineID& engineID, const STI::Engine::ParseID& parseID) override { }
	virtual void cancelTrigger() { }

	bool write(short channel, const STI::Utils::MixedValue& value);
	bool read(short channel, STI::Utils::MixedValue& data);
	bool read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
	void stopRW();


	bool writeChannelDefault(short channel, const STI::Utils::MixedValue& value);
	bool readChannelDefault(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
	bool playSingleEvent(const STI::Engine::RawEvent& event, std::shared_ptr<STI::Engine::ResultTicket>& resultTicket);

	void parseEventsDefault(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents);

	virtual bool isEventTarget(const DeviceID& id);

	void addCollectionListener(const std::shared_ptr<STI::Utils::LocalCollectionListenerAdapter<DeviceID>>& listener);

	std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& path, const std::string& filename);

	void setShotRepository(const std::shared_ptr<STI::Engine::ShotRepository>& repo);
	void setEngineConflictPolicy(const std::shared_ptr<STI::Engine::EngineConflictPolicy>& policy);

	std::string getAttribute(const std::string& key);
	bool setAttribute(const std::string& key, const std::string& value);
	bool refreshAttribute(const std::string& key);
	void refreshAttributes();
	bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute);


private:

	DeviceID normalizeEventTargetID(const DeviceID& id) const;

    void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher);

    void addMonitor(const std::shared_ptr<STI::Device::LocalMonitor>& monitor);
    void addMonitor(const std::string& id, std::shared_ptr<STI::Device::LocalMonitor>& monitor);

	virtual bool writeChannel(short channel, const STI::Utils::MixedValue& value) { return writeChannelDefault(channel, value); }
	virtual bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) { return readChannelDefault(channel, value, data); }

	friend class DeviceMessageListenerForwarder;
	void attachMessageListenerForwarder(const std::shared_ptr<DeviceMessageListenerForwarder>& forwarder) {}	//not needed for local device
	std::shared_ptr<DeviceMessageListenerForwarder> listenerForwarder;


	class DeviceCollectionListener : public STI::Utils::LocalCollectionListenerAdapter<DeviceID>
	{
	public:
		DeviceCollectionListener(LocalDevice* localDevice) : localDevice(localDevice) {}

		void add(const DeviceID& id);
		void remove(const DeviceID& id);

		LocalDevice* localDevice;
	};
	
	DeviceMessageListenerID schedulerMessageLID;
	
	std::shared_ptr<STI::Engine::ParseTicketManager<>> parseTicketManager;
	std::shared_ptr<STI::Engine::ResultTicketManager<>> resultTicketManager;

	friend DeviceCollectionPolicy;
	bool isPartnerDevice(const DeviceID& id);
	bool isTargetServerOf(const DeviceID& id);

	DeviceID id;
	STI::Utils::MetaData metaData;
	std::set<DeviceID> eventTargets;	//this LocalDevice can generate events for these (partner) devices
	bool usingParseDefault;
	bool usingRWdefault;
	
	virtual void setRemoveCB(const std::function<void(void)>& remover) override;
	std::function<void(void)> removerCallback;

	std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>> localCollection;
	std::shared_ptr<LocalDeviceMessageDispatcher> deviceMessageDispatcher;
	std::shared_ptr<DeviceMessageReceiver> deviceMessageReceiver;
	std::shared_ptr<STI::Engine::LocalEventEngineScheduler> eventEngineScheduler;
	std::shared_ptr<LocalChannelManager> localChannelManager;
	std::shared_ptr<LocalAttributeManager> localAttributeManager;
	std::shared_ptr<LocalPersistenceManager> localPersistenceManager;
	std::shared_ptr<LocalProfileManager> localProfileManager;
	std::shared_ptr<LocalTaskManager> localTaskManager;
	std::shared_ptr<LocalMonitorManager> localMonitorManager;
	std::shared_ptr<LocalLogManager> localLogManager;
	std::shared_ptr<VersionManager> versionManager;
	std::shared_ptr<LocalPostProcessingManager> localPostProcessingManager;

	std::shared_ptr<ServerMessageRelayer> serverMessageRelayer;

	std::set<DeviceID> partnerDevices;
	std::map<std::string, DeviceID> partnerAliases;

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
