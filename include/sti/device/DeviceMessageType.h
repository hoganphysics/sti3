#ifndef STI_DEVICE_DEVICEMESSAGETYPE_H
#define STI_DEVICE_DEVICEMESSAGETYPE_H


namespace STI
{
namespace Device
{


enum class DeviceMessageType { 
	Refresh, CollectionUpdate, 
	ChannelUpdate, ChannelsRefresh, 
	AttributeUpdate, AttributesRefresh, 
	MonitorUpdate, MonitorStatusUpdate,
	EngineJobUpdate,
	EngineScheduler,
	EngineParser,
	EngineStatus,
	PostProcessingComplete,
	Unknown };

//Adding a new DeviceMessageType:
//DeviceMessage.h: Add new derived class.
//DeviceMessageReceiver.h:  Add new type to each of the following:
//   DeviceMessageReceiver::addListener, ::removeListener, ::refreshListenerGroups, ::clearAllListenerGroups, 
//Add a dedicated ListenerGroupMap instance in DeviceMessageReceiver.


} //Device
} //STI


#endif

