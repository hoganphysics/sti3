import stidevicepy
import stipy

from stipy import event
from stipy import ch
from stipy import dev

import time



# id=stipy.HubID("Test","localhost",0)
did=stidevicepy.DeviceID("STI Server", "localhost", 0)
#print(type(did))
#print(did.getName())
#d1=dev("Test", "localhost",0)
#print(d1)
server=stipy.connect("localhost", did, "192.168.1.4:2809")


def testFunc2():
    print("Test func2")

def testFunc():
    # print("Test func")
    dd2=stipy.STIPyDevice("dev2", "localhost", 0, "localhost/0/STI Server")
    cc1=stipy.STIPyChannel(dd2, 1)
    event(cc1, 100, 55.0)
    # shot2=server.makeshot(testFunc2)


# server.cancelAll()

# print("sleep")
# time.sleep(0.25)
# print("wake")

shot=server.makeshot(testFunc)



d2=stipy.STIPyDevice("dev2", "localhost", 0, "localhost/0/STI Server")
c1=stipy.STIPyChannel(d2, 1)
c2=stipy.STIPyChannel(d2, 2)

d3=dev("dev2", "localhost", 0, "localhost/0/STI Server")
c3=ch(d3, 1)

shot.event(c3, 10, 37.4)
# shot.event(c1, 10, "Hello")
# shot.event(c2, 12, 22)
# shot.event(c1, 10, 2.0)
# event(c3, 200, 55.0)  #error

dServer=stipy.STIPyDevice("STI Server", "localhost", 0, "root")
chServer = stipy.STIPyChannel(dServer, 1)
shot.event(chServer, 15, 37.4)
# shot.event(chServer, 10, "Hello")

print(shot.getEvents())
# print(shot.getEvents()[0].getEventGraphPath())

tick=server.parse(shot)


try:
    tick.wait()
    # time.sleep(1.25)
except KeyboardInterrupt:
    print("Abort wait")
    tick.cancel()

errMessages=tick.getMessages()
print(errMessages)

#print(errMessages[0].getEvents()[0].type())

print(tick.getEvents())

# print(errMessages[0].getMessage())
# print(errMessages[0].getEvents())

playTick=server.play(tick)
playTick.wait()

# time.sleep(0.1)
