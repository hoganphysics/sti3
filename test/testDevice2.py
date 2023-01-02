import stipy
import stipy.stidevicepy as stidevicepy
import time


class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, name, address, module, targetServer):
        stidevicepy.LocalDevice.__init__(self, name, address, module, targetServer)

        # self.addChannel(1, stidevicepy.ChannelType.Output, stipy.MixedValueType.Empty, stipy.MixedValueType.Double, "test")
        self.addChannel(1, stidevicepy.ChannelType.Output, stipy.MixedValueType.Double, stipy.MixedValueType.Double, "test")
        self.addChannel(2, stidevicepy.ChannelType.Input, stipy.MixedValueType.Vector, stipy.MixedValueType.Double, "test2")
        self.addChannel(3, stidevicepy.ChannelType.Input, stipy.MixedValueType.Vector, stipy.MixedValueType.Vector, "test3")

        engineID = stidevicepy.EngineID(0)
        self.addEventEngine(engineID)

        self.addAttribute("test", "77").setSetter(self.testSetter)
        return
    def __del__(self):
        print("&&&&&&&&& TestDevice del")
    # def writeChannel(self, channel, value):
    #     print("write: " + str(channel))
    #     return True

    def testSetter(self, value):
        print('inside custom setter')
        if int(value) < 21:
            return True
        else:
            return False

    def parseEvents(self, eventsIn, synchedEvents):
        for key in eventsIn:
            for evt in eventsIn[key]:
                print("value = " + str(evt.value()))
                testEvent = TestDeviceEvent(35.8, evt.channel(), evt.value())
#                testEvent = TestDeviceEvent(35.8, evt.channel(), 456)

                if (evt.channel() == 2 or evt.channel() == 3):
                    testEvent.addMeasurement(evt)

                synchedEvents.append(testEvent)

        return

class TestDeviceEvent(stidevicepy.SynchronousEvent):
    def __init__(self, time, channel, value):
        stidevicepy.SynchronousEvent.__init__(self, time)
        self.channel = channel
        self.value = value
    def __del__(self):
        print("^^^^^^^^^^^ TestDeviceEvent del")
    def loadEvent(self):
        print("****** Custom load")
        return
    def collectMeasurementData(self):
        print("****** collectMeasurementData")
        if (self.channel == 2):
            tmpMeas = self.getMeasurements()
            for m in tmpMeas:
                print("++++++ collect 2")
                print("--> " + str(type(m)) + " in " + str(type(tmpMeas)))
                print(tmpMeas)
                val=stipy.MixedValue([123, 8, "test",[43,"abc"], 99.4])
                #val2=stidevicepy.MixedValue([43,"abc"])
#                val.addValue([43,"abc"])
                m.setMeasurementResult(val)
        if (self.channel == 3):
#            print("++++++ collect 3")
            tmpMeas = self.getMeasurements()
#            print("++++++ collect 3 -> 1")
            for m in tmpMeas:
                val=stipy.MixedValue([3,2,1])
#                val=stidevicepy.MixedValue([123, 8, "test",[43,"abc"], 99.4])
#                print("++++++ collect 3 -> 2")
                #val2=stidevicepy.MixedValue([43,"abc"])
#                val.addValue([43,"abc"])
                m.setMeasurementResult(val)
#                print("++++++ collect 3 -> 3")
        return
    def stopEvent(self):
        return
    def pauseEvent(self):
        return
    def unpauseEvent(self, retrigger):
        return
    def playEvent(self):
        print("Custom play event! val = " + str(self.value))
        return



dev1=TestDevice("TestDevice", "localhost", 0, "localhost/0/STI Server")
hub=stidevicepy.NetworkDeviceHub("192.168.1.4:2809")
# time.sleep(1)
dev1.write(1, 6.3)
tmp=dev1.read(2, 3.4)
print("result: "+str(tmp))

tmp=dev1.read(3, [6.3,4.4])
#tmp=dev1.read(3, 3.44)
#v = stidevicepy.MixedValue([43,"abc"])
#print("py val = " + str(v))
#tmp=dev1.read(3, v)
print("result: "+str(tmp))

manager = dev1.getAttributeManager()
manager.setValue("test","20")
print("Attribute test=" + str(manager.getValue("test")) )

hub.addDevice(dev1)

hub.run(True)
hub.shutdown()

# dev1 = 0
# time.sleep(0.1)
# exit()
