import stipy
from testDevice import *
import time

config = stipy.Configuration(
    {'Device Name': 'TestParseDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'sr-magis/2/Frame2'})

device = TestDevice(config)

# res2 = device.read(2)
# print("Read result=" + str(res2))

# res2 = device.read(2)
# print("Read result=" + str(res2))


def f():
    stipy.meas(stipy.ch(stipy.dev("TestDevice", "localhost", 0), 2), 200*1000)
    stipy.meas(stipy.ch(stipy.dev("TestDevice", "localhost", 0), 3), 300*1000)

    stipy.event(stipy.ch(stipy.dev("TestDevice", "localhost", 0), 1), 200*1000 + 1*200*1000, 5.7)
    return




nameServiceAddr = "192.168.88.252:2809"   #OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(device)

# hub.run(False)
hub.run(True)

testDevice = stipy.connect("localhost/0/TestDevice", "192.168.1.4:2809")
s = testDevice.makeshot(f)
tick = testDevice.parse(s)
tick.wait()
print("       Messages: " + str(tick.messages()))

res = testDevice.play(tick)
res.wait()
print("       Results: " + str(res.measurements()))
