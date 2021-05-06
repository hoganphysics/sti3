#ifndef STI_NETWORK_REMOTEDEVICE_H
#define STI_NETWORK_REMOTEDEVICE_H

#include "deviceNet.h"

#include "Device.h"
#include "DeviceCollection.h"
#include "TDeviceRefInterface.h"
#include "DeviceMessageDispatcher.h"
#include "fwd/ChannelManager_fwd.h"

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



class RemoteDevice : public STI::Device::Device, 
					 public STI::Network::TDeviceRefInterface	//mixin
{
public:

	RemoteDevice(::STI::TNetwork::TDevice_ptr device);

	bool refresh();

	const STI::Device::DeviceID getID() const;
	
	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
	void getChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager);
	void getAttributeManager(std::shared_ptr<STI::Device::AttributeManager>& manager);

private:

	template<typename T>
	bool isLive(const std::shared_ptr<T>& remote)
	{
		return (remote != 0 && remote->ping());
	}

	void attachMessageListenerForwarder(const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder);

	bool getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice);

	::STI::TNetwork::TDevice_var _tDevice;		//remote reference

	std::shared_ptr<STI::Device::DeviceMessageListenerForwarder> listenerForwarder;

	std::shared_ptr<RemoteDeviceCollection> remoteCollection;
	std::shared_ptr<RemoteDeviceMessageDispatcher> remoteDispatcher;
	std::shared_ptr<RemoteEventEngineScheduler> remoteScheduler;
	std::shared_ptr<RemoteChannelManager> remoteChannelManager;
	std::shared_ptr<RemoteAttributeManager> remoteAttributeManager;

	mutable std::mutex deviceMutex;

};


} //Network
} //STI


#endif

