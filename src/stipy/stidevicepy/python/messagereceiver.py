from stipy.bin.stidevicepy import DeviceMessageReceiver
from stipy.bin.stidevicepy import DeviceMessage
from stipy.bin.stidevicepy import DeviceMessageType
from stipy.bin.stipybase import DeviceID

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
    elif type == DeviceMessageType.EngineJobUpdate:
        return self.__addEngineJobUpdateDeviceListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.EngineScheduler:
        return self.__addEngineSchedulerMessageListener(sourceDeviceID, listenerName, handler)
    elif type == DeviceMessageType.EngineStatus:
        return self.__addEngineStateMessageListener(sourceDeviceID, listenerName, handler)
    return

setattr(DeviceMessageReceiver, 'addListener', addListener)
