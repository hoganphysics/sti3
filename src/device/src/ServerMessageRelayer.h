#ifndef STI_DEVICE_SERVERMESSAGERELAYER_H
#define STI_DEVICE_SERVERMESSAGERELAYER_H


#include "DeviceMessageRelayer.h"
#include "DeviceMessage.h"


namespace STI
{
namespace Device
{


typedef DeviceMessageRelayer<ChannelUpdateMessage, 
                             AttributeUpdateMessage,
                            //  EngineSchedulerMessage, 
                             RefreshDeviceMessage, 
                             CollectionUpdateMessage,
                             EngineStateMessage,
                             EngineJobUpdateDeviceMessage> ServerMessageRelayer;


} //Device
} //STI

#endif

