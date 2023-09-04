#ifndef STI_NETWORK_REMOTEDEVICE_H
#define STI_NETWORK_REMOTEDEVICE_H

#include "generated/deviceNet.h"

#include <sti/device/Device.h>
#include <sti/device/DeviceCollection.h>
#include "TDeviceRefInterface.h"
#include <sti/device/DeviceMessageDispatcher.h>
#include "fwd/ChannelManager_fwd.h"
#include <sti/utils/CachedValue.h>

#include "TReferenceHolder.h"

#include <memory>
#include <mutex>

namespace STI
{
namespace Network
{


class RemoteDeviceCollection;
class RemoteDeviceMessageDispatcher;
class RemoteEventEngineScheduler;
class RemoteChannelManager;
class RemoteAttributeManager;
class RemotePersistenceManager;
class RemoteProfileManager;
class RemoteLogManager;


class RemoteDevice : public STI::Device::Device, 
					 public STI::Network::TDeviceRefInterface,	//mixin
					 public STI::TNetwork::TReferenceHolder<STI::TNetwork::TDevice>	//mixin
{
public:

	RemoteDevice(::STI::TNetwork::TDevice_ptr device);
	~RemoteDevice();

	bool refresh();
	void kill();
	void activate();
	void disable();
	
	const STI::Device::DeviceID getID() const;	//use locally stored value
	
	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<STI::Device::AttributeManager>& manager);
	bool getPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& manager);
	bool getProfileManager(std::shared_ptr<STI::Device::ProfileManager>& manager);
	bool getLogManager(std::shared_ptr<STI::Device::LogManager>& manager);

	bool write(short channel, const STI::Utils::MixedValue& value);
	bool read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
	void stopRW();
	std::string getAttribute(const std::string& key);
	bool setAttribute(const std::string& key, const std::string& value);
	bool getAttribute(const std::string& key, std::shared_ptr<STI::Device::Attribute>& attribute);

private:

	template<typename T>
	bool isLive(const std::shared_ptr<T>& remote)
	{
		return (remote != 0 && !remote->isDisabled() && remote->ping());
	}

	void attachMessageListenerForwarder(const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder);

//	bool getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice);
	bool getTDeviceRef(STI::TNetwork::TDevice_var& tDevice);

//	::STI::TNetwork::TDevice_var _tDevice;		//remote reference

	std::shared_ptr<STI::Device::DeviceMessageListenerForwarder> listenerForwarder;

	std::shared_ptr<RemoteDeviceCollection> remoteCollection;
	std::shared_ptr<RemoteDeviceMessageDispatcher> remoteDispatcher;
	std::shared_ptr<RemoteEventEngineScheduler> remoteScheduler;
	std::shared_ptr<RemoteChannelManager> remoteChannelManager;
	std::shared_ptr<RemoteAttributeManager> remoteAttributeManager;
	std::shared_ptr<RemotePersistenceManager> remotePersistenceManager;
	std::shared_ptr<RemoteProfileManager> remoteProfileManager;
	std::shared_ptr<RemoteLogManager> remoteLogManager;

	mutable STI::Utils::CachedValue<STI::Device::DeviceID> cachedDeviceID;

	mutable std::mutex deviceMutex;

};


} //Network
} //STI


#endif

