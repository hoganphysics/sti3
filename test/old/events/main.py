import threading
import stipy
import stipy.stidevicepy as stidevicepy


class EventChecker(stidevicepy.LocalDevice):
    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)
        self.events = []
        self._lock = threading.Lock()
        self.addOutputChannel(0, stipy.MixedValueType.Number, "Start Event Checker")
        self.addOutputChannel(1, stipy.MixedValueType.Number, "End Event Checker")

        serverID = stipy.DeviceID("localhost/0/STI Server")
        self.addPartner(serverID, "server")

        receiver = self.getMessageReceiver()

        receiver.addListener(stipy.DeviceMessageType.EngineScheduler, serverID, "EngineSchedulerUpdates", self.schedulerUpdates)

    def schedulerUpdates(self, message) :
        print("schedulerUpdates!")
        print(message)

    def writeChannel(self, channel, value):
        print("test")
        with self._lock:
            if channel == 0:
                self.events = []
            elif channel == 1:
                print(self.events)
            
        return True
    
    def onEvent(self, event):
        with self._lock:
            self.events.append(event)


class EventCheckingDevice(stidevicepy.LocalDevice):
    def __init__(self, config, checker: EventChecker):
        stidevicepy.LocalDevice.__init__(self, config)
        self.checker = checker
        self.addAttribute("Enable Trigger", "On", ["On", "Off"])
        return
    
    def writeChannel(self, channel, value):
        print(f"Test {channel} value: {value}")
        # self.checker.onEvent(f"Write to device {self.getID().getID()} on channel {channel} with value {value}")
        return True
    
    def readChannel(self, channel, value):
        return None

###################################


nameServiceAddr = "192.168.88.252:2809"   #OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

dev_config = {'Device Name': 'EventCheckingDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'sr-magis/2/Frame2'}


checker = EventChecker(dev_config)
hub.addDevice(checker)

dev_config['Device Name'] = 'Device1'
device1 = EventCheckingDevice(stipy.Configuration(dev_config), checker)
device1.addOutputChannel(0, stipy.MixedValueType.Double, "")
device1.addOutputChannel(1, stipy.MixedValueType.Double, "")
device1.addVersionInfo("Event Checking Device", "1.0.0")
hub.addDevice(device1)

dev_config['Device Name'] = 'Device2'
device2 = EventCheckingDevice(stipy.Configuration(dev_config), checker)
device2.addOutputChannel(0, stipy.MixedValueType.Double, "")
hub.addDevice(device2)

dev_config['Device Name'] = 'Device3'
device3 = EventCheckingDevice(stipy.Configuration(dev_config), checker)
device3.addOutputChannel(0, stipy.MixedValueType.Double, "")
device3.addOutputChannel(1, stipy.MixedValueType.Double, "")
device3.addOutputChannel(2, stipy.MixedValueType.Double, "")
hub.addDevice(device3)


hub.run(True)
