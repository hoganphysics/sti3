from stipy import *

testDevice = connect(DeviceID("TestDevice", "localhost", 0), "192.168.1.7:2809")

print(testDevice.write(0, 12.4))

exit()
