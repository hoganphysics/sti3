#ifndef STI_DEVICE_SERVERMESSAGERELAYER_H
#define STI_DEVICE_SERVERMESSAGERELAYER_H


#include <sti/device/DeviceMessageRelayer.h>
#include <sti/device/DeviceMessage.h>


namespace STI
{
namespace Device
{


typedef DeviceMessageRelayer<ChannelUpdateMessage, 
                             AttributeUpdateMessage,
                             MonitorUpdateMessage,
                             MonitorStatusUpdateMessage,
                            //  EngineSchedulerMessage, 
                             RefreshDeviceMessage, 
                             CollectionUpdateMessage,
                             EngineStateMessage,
                             EngineJobUpdateDeviceMessage> ServerMessageRelayer;


} //Device
} //STI

#endif

