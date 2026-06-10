import stipy
from testDevice import *
import time


class TestListenerDevice(stidevicepy.LocalDevice):
    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)
        
        sourceID = stipy.DeviceID("localhost/0/TestDevice")     # the origin of the messages we want to listen to

        self.addPartner(sourceID)   # must declare the message source device as a partner to capture messages

        # *** Define listeners *** #
        receiver = self.getMessageReceiver()

        receiver.addListener(stipy.DeviceMessageType.ChannelUpdate, sourceID, "PythonChannelUpdates", self.channelUpdateListener)
        receiver.addListener(stipy.DeviceMessageType.AttributeUpdate, sourceID, "PythonAttributeUpdates", self.attributesUpdateListener)

        serverID = stipy.DeviceID("sr-magis/2/Frame2")     # the origin of the messages we want to listen to
        self.addPartner(serverID)
        receiver.addListener(stipy.DeviceMessageType.EngineJobUpdate, serverID, "PythonEngineJobUpdate", self.engineJobUpdateListener)
        self.count = 0
        receiver.addListener(stipy.DeviceMessageType.EngineStatus, serverID, "PythonEngineStatusUpdates", self.engineStatusListener)
        self.count2 = 0

        # def update(stipy.DeviceID("localhost/0/TestDevice")
        return
    
    def channelUpdateListener(self, message) :
        print("Channel update!")
        print("values:", message.channelValues())
        print("measurements:", message.measurementValues())
    
    def attributesUpdateListener(self, message) :
        print("Attribute update!")
        print(message.attributes)

    def engineJobUpdateListener(self, message) :
        print("count =", self.count)
        self.count += 1
    def engineStatusListener(self, message) :
        print("count2 =", self.count2)
        self.count2 += 1


config1 = stipy.Configuration(
    {'Device Name': 'TestDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'sr-magis/2/Frame2'})

sourceDevice = TestDevice(config1)


config2 = stipy.Configuration(
    {'Device Name': 'TestListenerDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'sr-magis/2/Frame2'})

listenerDevice = TestListenerDevice(config2)

nameServiceAddr = "192.168.1.109:2809"   #OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(sourceDevice)
hub.addDevice(listenerDevice)

# hub.run(False)
hub.run(True)

time.sleep(1)

sourceDevice.write(0, 5.1)
