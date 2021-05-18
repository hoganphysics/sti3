#ifndef STI_NETWORK_REMOTEDEVICEMESSAGEDISPATCHER_H
#define STI_NETWORK_REMOTEDEVICEMESSAGEDISPATCHER_H


#include "DeviceMessageDispatcher.h"
#include "DeviceID.h"

#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace Network
{

class RemoteDeviceMessageDispatcher : public STI::Device::DeviceMessageDispatcher
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

	::STI::TNetwork::TDeviceMessageDispatcher_var tMessageDispatcher;		//remote reference

};


} //Network
} //STI


#endif

