import stipy
from testDevice import *
from testDevice2 import *
import time

config1 = stipy.Configuration(
    {'Device Name': 'TestDevice',
     'IP Address': 'localhost',
     'Module': '1',
     'Target Server': 'sr-magis/2/Frame2'})

config2 = stipy.Configuration(
    {'Device Name': 'TestDevice2',
     'IP Address': 'localhost',
     'Module': '2',
     'Target Server': 'sr-magis/2/Frame2'})

device1 = TestDevice(config1)
device2 = TestDevice2(config2)



nameServiceAddr = "192.168.1.109:2809"   #OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(device1)
hub.addDevice(device2)

# hub.run(False)
hub.run(True)
