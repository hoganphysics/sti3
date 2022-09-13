from stipy.stidevicepy import *


class SimpleDevice(LocalDevice):
    def __init__(self, name, address, module, targetServer):
        LocalDevice.__init__(self, name, address, module, targetServer)


device = SimpleDevice("Simple Device", "localhost", 1, "localhost/0/STI Server")

hub = NetworkDeviceHub("192.168.1.4:2809")

hub.addDevice(device)
hub.run()

