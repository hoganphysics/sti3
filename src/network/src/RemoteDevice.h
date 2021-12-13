#ifndef STI_NETWORK_REMOTEDEVICE_H
#define STI_NETWORK_REMOTEDEVICE_H

#include "deviceNet.h"

#include "Device.h"
#include "DeviceCollection.h"
#include "TDeviceRefInterface.h"
#include "DeviceMessageDispatcher.h"
#include "fwd/ChannelManager_fwd.h"

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


class RemoteDevice : public STI::Device::Device, 
					 public STI::Network::TDeviceRefInterface,	//mixin
					 public STI::TNetwork::TReferenceHolder<STI::TNetwork::TDevice>	//mixin
{
public:

	RemoteDevice(::STI::TNetwork::TDevice_ptr device);
	~RemoteDevice();

	bool refresh();
	void kill();
	void disable();
	
	const STI::Device::DeviceID getID() const;	//use locally stored value
	
	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<STI::Device::AttributeManager>& manager);
	bool getPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& manager);

private:

	template<typename T>
	bool isLive(const std::shared_ptr<T>& remote)
	{
		return (remote != 0 && !remote->isDisabled() && remote->ping());
	}

	void loadDeviceID();	//remote call

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

	STI::Device::DeviceID deviceID;

	mutable std::mutex deviceMutex;

};


} //Network
} //STI


#endif

