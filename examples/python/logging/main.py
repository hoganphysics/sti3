import stipy
from testDevice import *
import time



config = stipy.Configuration(
    {'Device Name': 'TestDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'sr-magis/2/Frame2'})

testDevice = TestDevice(config)


nameServiceAddr = "192.168.1.109:2809"   #OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(testDevice)

# hub.run(False)
hub.run(True)

# lm=testDevice.getLogManager()
# lm.getLogRecord("2023/10/31")

# filter=stipy.LogFileFilter()
# filter.logName = "testing"
# filter.startDate="2023/10/30"
# filter.endDate="2023/10/31"
# filter.startIndex = 0
# filter.endIndex = 10
# print(filter)

# print(lm.getLogCount(filter))
