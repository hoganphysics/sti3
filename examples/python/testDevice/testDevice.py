import stipy
import stipy.stidevicepy as stidevicepy


class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, name, address, module, targetServer):
        stidevicepy.LocalDevice.__init__(self, name, address, module, targetServer)

        self.addChannel(1, stidevicepy.ChannelType.Output, stipy.MixedValueType.Empty, stipy.MixedValueType.Double, "test")
        self.addChannel(2, stidevicepy.ChannelType.Input, stipy.MixedValueType.Double, stipy.MixedValueType.Double, "test2")

        engineID = stidevicepy.EngineID(0)
        self.addEventEngine(engineID)
        return
    def __del__(self):
        print("&&&&&&&&& TestDevice del")
    def writeChannel(self, channel, value):
         print("write: " + str(channel))
         return True


    def parseEvents(self, eventsIn, synchedEvents):
        
        for key in eventsIn:
            for evt in eventsIn[key]:
                testEvent = TestDeviceEvent(35.8)
                synchedEvents.append(testEvent)

        return

class TestDeviceEvent(stidevicepy.SynchronousEvent):
    def __init__(self, time):
        stidevicepy.SynchronousEvent.__init__(self, time)
    def __del__(self):
        print("^^^^^^^^^^^ TestDeviceEvent del")
    def loadEvent(self):
        print("****** Custom load")
        return
    def collectMeasurementData(self):
        return
    def stopEvent(self):
        return
    def pauseEvent(self):
        return
    def unpauseEvent(self, retrigger):
        return
    def playEvent(self):
        print("Custom play event!")
        return



dev1=TestDevice("TestDevice 2", "maximus", 1, "localhost/0/BridgeServer")

hub=stidevicepy.NetworkDeviceHub("192.168.1.4:2809")

hub.addDevice(dev1)

hub.run(True)

dev1 = 0
