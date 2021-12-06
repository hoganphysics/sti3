import stidevicepy
import faulthandler
import time
import sys
import gc

# print(stidevicepy.ChannelType.Output)


class B(stidevicepy.A):
    def __init__(self, val):
        super(B, self).__init__(val)
        # stidevicepy.A.__init__(self, val)
    def run(self):
        print("B:" + str(self.val))
        return

class TT:
    def __init__(self, val):
        self.val = val
    

class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, name, address, module, targetServer):
        stidevicepy.LocalDevice.__init__(self, name, address, module, targetServer)

        self.addChannel(1, stidevicepy.ChannelType.Output, stidevicepy.MixedValueType.Empty, stidevicepy.MixedValueType.Double, "test")
        self.addChannel(2, stidevicepy.ChannelType.Input, stidevicepy.MixedValueType.Double, stidevicepy.MixedValueType.Double, "test2")

        engineID = stidevicepy.EngineID(0)
        self.addEventEngine(engineID)
        return
    def __del__(self):
        print("&&&&&&&&& TestDevice del")
    # def writeChannel(self, channel, value):
    #     print("write: " + str(channel))
    #     return True
    def testVector(self, inputVec):
        print(type(inputVec))
        print(inputVec)
        inputVec.append(12)
        print(inputVec)
        print(len(inputVec))
        [print("val=" + str(x)) for x in inputVec]
        return
    def testVector2(self, inputVec):
        print(type(inputVec))
        print(inputVec)
        #a = stidevicepy.A(13)
        a = B(13)
        inputVec.append(a)
        b = B(88)
        inputVec.append(b)
        self.saveVec = []
        self.saveVec.append(a)
        self.saveVec.append(b)
        print(inputVec)
        print(len(inputVec))
        [x.run() for x in inputVec]
        return
    # def parseEvents(self, eventsIn, synchedEvents):
    #     self.saveSync=[]
    #     tmp = self.parseEvents2(eventsIn, synchedEvents)
    #     [self.saveSync.append(x) for x in synchedEvents]
    #     return
    def parseEvents(self, eventsIn, synchedEvents):
        #synchedEvents=[]
        self.saveSync=[]
        
        for key in eventsIn:
            # print(str(eventsIn.keys()))
            # print(str(key))
            # print(str(eventsIn[key]))
            print(type(synchedEvents))

            for evt in eventsIn[key]:
                print(evt)
                testEvent = TestDeviceEvent(35.8)
                print("testEvent ref count = " + str(sys.getrefcount(testEvent)))
                # testEvent.play()
                # synchedEvents=[testEvent]
                synchedEvents.append(testEvent)
                testEvent2 = TestDeviceEvent(36.8)
                synchedEvents.extend([testEvent2])
                # synchedEvents[0]=testEvent2
                # testEvent=0
                # self.saveEvt=testEvent
                print("testEvent ref count = " + str(sys.getrefcount(testEvent)))
                
                
                # synchedEvents.clear()
                # self.saveSync.append(testEvent)
                #synchedEvents[0].play()
                # print("synchedEvents len="+str(len(synchedEvents)))
        # synchedEvents.clear()
        # self.saveSync = synchedEvents
        # [self.saveSync.append(x) for x in synchedEvents]
        return

class TestDeviceEvent(stidevicepy.SynchronousEvent):
    # def __new__(cls, time):
    #     print("TestDeviceEvent new!")
    #     tmp = stidevicepy.SynchronousEvent.__new__(cls, time)
    #     # tmp.__class__ = stidevicepy.SynchronousEvent
    #     # tmp.add2()
    #     # tmp.__class__ = TestDeviceEvent
    #     return tmp
    #     # return super(TestDeviceEvent, cls).__new__(cls)
    def __init__(self, time):
        print("TestDeviceEvent init!")
        stidevicepy.SynchronousEvent.__init__(self, time)
    def __del__(self):
        print("^^^^^^^^^^^ TestDeviceEvent del")
    def loadEvent(self):
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



# a = stidevicepy.A(13)
# a.run()

# b = B(73)
# b.run()

tt = TT(44)
print("tt ref count = " + str(sys.getrefcount(tt)))

dev1=TestDevice("TestDevice", "localhost", 0, "root")

print("dev1 ref count = " + str(sys.getrefcount(dev1)))

# dev1.runTest2()
# exit()


faulthandler.enable()
res = dev1.write(1, 3.3)
print("Write result=" + str(res))

# time.sleep(0.1)

# dev1.write(1, 6.3)

# b=6


# print("DONE testEvent ref count = " + str(sys.getrefcount(dev1.saveEvt)))



print("dev1 ref count = " + str(sys.getrefcount(dev1)))

dev1 = 0


gc.collect()


# referrers = gc.get_referrers(dev1)
# print("referrers: ")
# for referrer in referrers:
#     # print(namestr(referrer, locals()))
#     print(str(referrer['dev1']))
time.sleep(0.1)

exit()

# res = dev1.read(1, 5.3)
# print("Read result=" + str(res))

at=stidevicepy.LocalAttribute("g::p::test","34")

def testSetter(value):
    print('inside custom setter')
    if value == '20':
        return True
    else:
        return False


def testRefresher():
    print('inside refresher')
    return '77'


# print(testSetter('20'))
# print(testSetter('21'))

print(at.getValue())
# at.setValue('31')
# print(at.getValue())

at.setSetter(testSetter)

# at.setSetter(testSetter).setRefresher(testRefresher)
# at.setRefresher(testRefresher)
at.setValue('22')

print(at)

dev1.addAttribute("x", "77").setSetter(testSetter)
manager = dev1.getAttributeManager()
manager.setValue("x","20")
print(manager.getValue("x"))

# print(tmp)
