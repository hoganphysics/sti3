#ifndef STI_DEVICE_DEVICEMESSAGEDISPATCHER_H
#define STI_DEVICE_DEVICEMESSAGEDISPATCHER_H


#include <memory>

namespace STI
{
namespace Device
{

class DeviceID;
class DeviceMessage;
class DeviceMessageHandler;


class DeviceMessageDispatcher
{
public:
	
	virtual ~DeviceMessageDispatcher() {}

	virtual void addMessageHandler(const DeviceID& targetID, const std::shared_ptr<DeviceMessageHandler>& handler) = 0;
	virtual void removeMessageHandler(const DeviceID& targetID) = 0;
	virtual bool makeMessageHandler(std::shared_ptr<DeviceMessageHandler>& handler) = 0;

	virtual void addMessage(const std::shared_ptr<DeviceMessage>& mess) = 0;
	virtual void clearMessages() = 0;

};


} //Device
} //STI


#endif

