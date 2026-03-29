from stipy.stidevicepy.stidevicepy import DeviceMessageReceiver
from stipy.stidevicepy.stidevicepy import DeviceMessage
from stipy.stidevicepy.stidevicepy import DeviceMessageType
from stipy.stipybase.stipybase import DeviceID

from typing import Callable

def addListener(self, type: DeviceMessageType, sourceDeviceID: DeviceID, listenerName: str, handler: Callable) :
    if type == DeviceMessageType.Refresh:
        return self.__addRefreshListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.CollectionUpdate:
        return self.__addCollectionUpdateListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.ChannelUpdate:
        return self.__addChannelUpdateListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.AttributeUpdate:
        return self.__addAttributeUpdateListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.MonitorUpdate:
        return self.__addMonitorUpdateListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.MonitorStatusUpdate:
        return self.__addMonitorStatusUpdateListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.EngineJobUpdate:
        return self.__addEngineJobUpdateDeviceListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.EngineScheduler:
        return self.__addEngineSchedulerMessageListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.EngineStatus:
        return self.__addEngineStateMessageListener(sourceDeviceID, listenerName, handler)
    return

setattr(DeviceMessageReceiver, 'addListener', addListener)
