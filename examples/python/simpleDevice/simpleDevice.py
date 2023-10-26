from stipy.stidevicepy import *

# Define SimpleDevice class, the minimum working STI device:

class SimpleDevice(LocalDevice):
    def __init__(self, name, address, module, targetServer):
        LocalDevice.__init__(self, name, address, module, targetServer)


##################################

#Make a SimpleDevice instance and connect it to the STI network

device = SimpleDevice("Simple Device", "localhost", 0, "localhost/0/STI Server")

nameServiceAddr = "192.168.1.4:2809"   #OmniORB NameService
hub = NetworkDeviceHub(nameServiceAddr)

hub.addDevice(device)
hub.run()

