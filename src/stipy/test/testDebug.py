from stipy import *

testDevice = connect(DeviceID("TestDevice 2", "localhost", 1), "171.64.56.68:2809")

pm=testDevice.getProfileManager()

pm.getProfiles()

p=pm.getProfile("safe")


testDevice.getProfileManager()

