import stidevicepy
import stipy

from stipy import event
from stipy import ch
from stipy import dev

import time


# id=stipy.HubID("Test","localhost",0)
did=stidevicepy.DeviceID("STI Server", "localhost", 0)
server=stipy.connect("localhost", did, "192.168.1.6:2809")


def testFunc():
    # print("Test func")
    dd2=stipy.STIPyDevice("dev2", "localhost", 0, "localhost/0/STI Server")
    cc1=stipy.STIPyChannel(dd2, 1)
    event(cc1, 100, 55.0)


shot=server.makeshot(testFunc)


#time.sleep(0.1)
