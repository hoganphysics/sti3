
##Need to run
# export LD_LIBRARY_PATH=.
#From cmd line before package will work.
# LD_LIBRARY_PATH tells python where to look for *.so files.
# Here we just add the local dir temporarily...
#https://stackoverflow.com/questions/1099981/why-cant-python-find-shared-objects-that-are-in-directories-in-sys-path

# from setuptools import setup

# setup(
#     name='libstidevice',
#     version=3,  # specified elsewhere
#     package_dir={'': '.'},
#     package_data={'': ['libstidevice.so']}
# )

#from example import *
#import example
import stidevicepy

class TestDevice(stidevicepy.LocalDevice2) :
    def writeChannel(self, channel, value) :
        print("py write channel = " + str(value))
        return True
    def readChannel(self, channel, value) :
        print("py read channel = " + str(value))
        return 3*value

dev=stidevicepy.LocalDevice2("dev","localhost",0, "srv")
print(dev.writeChannel(0, 8))


tdev=TestDevice("DevT","localhost",12, "srv")

tdev.addChannel(0)

# print(tdev.getID())
print(tdev.writeChannel(0,4))

man=tdev.getChannelManager()
# val=stidevicepy.MixedValue(44)
# man.writeChannel(0,val)

print(man.getChannels())
print(man.getChannel(0).getChannelName())
print(man.writeChannel(0,6))

print(man.readChannel(0,8))


dev2=stidevicepy.LocalDevice2("dev2","localhost",0, "srv")

dev.addPartner(dev2.getID())

collection=dev.getDeviceCollection()
#print(collection.getIDs())
collection.add(dev2.getID(), dev2)
print(collection.contains(dev2.getID()))
print(collection.get(dev2.getID()).getID())
print(collection.size())
print(collection.getIDs())



id=stidevicepy.DeviceID("dev","localhost",0)
evt=stidevicepy.RawEvent(id, 3.5, 0, 45, "desc", 2, stidevicepy.RawEventType.Play)

print(evt.value())
print(evt.getEventGraphPath())
print(evt)
