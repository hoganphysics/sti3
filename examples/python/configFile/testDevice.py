import stipy
import stipy.stidevicepy as stidevicepy


class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)

        self.addVersionInfo("TestDevice", "1.0.0")

        name = config.get("Device Name")
        print("Configuring " + name) 

        try:
            x = config.get("SetupData", "x")
            print("x = " + x)
        except KeyError:
            print("x not found!")

        try:
            y = config.get("SetupData", "y")
            print("y = " + y)
        except KeyError:
            print("y not found!")

        states = config.getList("SetupData", "states")
        print("states: " + str(states))

        listExample = config.getList("SetupData", "example")
        print("example: " + str(listExample))

        return



configFile = stipy.ConfigFile("testDevice.ini")

device1 = TestDevice(configFile.extract("TestDevice1"))
device2 = TestDevice(configFile.extract("TestDevice2"))

hub = stidevicepy.NetworkDeviceHub(configFile);     #Automatically uses [NetworkHub] parameters to configure hub

hub.addDevice(device1)
hub.addDevice(device2)

hub.run(True)

