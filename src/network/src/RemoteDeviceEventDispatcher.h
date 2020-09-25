#ifndef STI_NETWORK_REMOTEDEVICEEVENTDISPATCHER_H
#define STI_NETWORK_REMOTEDEVICEEVENTDISPATCHER_H


#include "DeviceEventDispatcher.h"
#include "DeviceID.h"

#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace Network
{

class RemoteDeviceEventDispatcher : public STI::Device::DeviceEventDispatcher
{
public:

	RemoteDeviceEventDispatcher(::STI::TNetwork::TDeviceEventDispatcher_ptr eventDispatcher);
	~RemoteDeviceEventDispatcher();

	void addEventHandler(const STI::Device::DeviceID& targetID, const std::shared_ptr<STI::Device::DeviceEventHandler>& handler);
	void removeEventHandler(const STI::Device::DeviceID& targetID);
	bool makeEventHandler(std::shared_ptr<STI::Device::DeviceEventHandler>& handler);

	void addEvent(const std::shared_ptr<STI::Device::DeviceEvent>& evt);
	void clearEvents();


private:

	::STI::TNetwork::TDeviceEventDispatcher_var tEventDispatcher;		//remote reference

};


} //Network
} //STI


#endif

