#ifndef STI_DEVICE_DEVICEMESSAGEHANDLER_H
#define STI_DEVICE_DEVICEMESSAGEHANDLER_H

//#include "DeviceMessage.h"

#include <memory>

namespace STI
{
namespace Device
{

enum class DeviceMessageType;
class AbstractMessageListenerGroup;
class DeviceMessage;

class DeviceMessageHandler
{
public:

	virtual ~DeviceMessageHandler() {}

	virtual void addListenerGroup(const DeviceMessageType& type, 
		std::shared_ptr<AbstractMessageListenerGroup>& listenerGroup) = 0;
	virtual void removeListenerGroup(const DeviceMessageType& type) = 0;

	virtual void addMessage(const std::shared_ptr<DeviceMessage>& mess) = 0;
	virtual void clearMessages() = 0;

	virtual bool hasListeners(const std::shared_ptr<DeviceMessage>& mess) = 0;

};


} //Device
} //STI


#endif

