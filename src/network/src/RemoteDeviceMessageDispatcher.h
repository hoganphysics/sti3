#ifndef STI_NETWORK_REMOTEDEVICEMESSAGEDISPATCHER_H
#define STI_NETWORK_REMOTEDEVICEMESSAGEDISPATCHER_H


#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/DeviceID.h>

#include "deviceNet.h"
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>


namespace STI
{
namespace Network
{


class RemoteDeviceMessageDispatcher : public STI::Device::DeviceMessageDispatcher,
									  public STI::TNetwork::TReferenceHolder<STI::TNetwork::TDeviceMessageDispatcher>	//mixin
{
public:

	RemoteDeviceMessageDispatcher(::STI::TNetwork::TDeviceMessageDispatcher_ptr messageDispatcher);
	~RemoteDeviceMessageDispatcher();

	void addMessageHandler(const STI::Device::DeviceID& targetID, const std::shared_ptr<STI::Device::DeviceMessageHandler>& handler);
	void removeMessageHandler(const STI::Device::DeviceID& targetID);
	bool makeMessageHandler(std::shared_ptr<STI::Device::DeviceMessageHandler>& handler);

	void addMessage(const std::shared_ptr<STI::Device::DeviceMessage>& mess);
	void clearMessages();

	bool ping() const;

private:

	mutable std::mutex dispatcherMutex;
	//::STI::TNetwork::TDeviceMessageDispatcher_var tMessageDispatcher;		//remote reference

};


} //Network
} //STI


#endif

