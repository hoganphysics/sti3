from stipy.stidevicepy.stidevicepy import DeviceCollection
from stipy.stipybase.stipybase import DeviceID


_get = DeviceCollection.get

def get(self, deviceID) :
    if isinstance(deviceID, str) :
        id = DeviceID(deviceID)
        return _get(self, id)
    else:
        return _get(self, deviceID)

setattr(DeviceCollection, 'get', get)

