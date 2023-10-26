import stipy
from testDevice import *

config = stipy.Configuration(
    {'Device Name': 'TestDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'localhost/0/STI Server'})

device = TestDevice(config)

# res2 = device.read(2)
# print("Read result=" + str(res2))

# res2 = device.read(2)
# print("Read result=" + str(res2))

nameServiceAddr = "192.168.1.4:2809"   #OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(device)

hub.run(True)
