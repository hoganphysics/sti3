from stipy.bin.stidevicepy import DeviceCollection
from stipy.bin.stipybase import DeviceID


_get = DeviceCollection.get

def get(self, deviceID) :
    if isinstance(deviceID, str) :
        id = DeviceID(deviceID)
        return _get(self, id)
    else:
        return _get(self, deviceID)

setattr(DeviceCollection, 'get', get)

