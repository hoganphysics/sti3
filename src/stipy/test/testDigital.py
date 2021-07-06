import stidevicepy
import stipy

from stipy import event
from stipy import ch
from stipy import dev

import time


#did=stidevicepy.DeviceID("STI Server", "localhost", 0)
did=stidevicepy.DeviceID("Digital Out", "localhost", 0)
server=stipy.connect("localhost", did, "192.168.1.2:2809")


def testFunc():
    # print("Test func")
    digital=stipy.STIPyDevice("Digital Out", "localhost", 0, "localhost/0/STI Server")
    event(ch(digital, 0), 100, True)
    event(ch(digital, 0), 140, False)
    event(ch(digital, 0), 180, True)
    event(ch(digital, 1), 150, True)
    event(ch(digital, 1), 200, False)


shot=server.makeshot(testFunc)

tick=server.parse(shot)

try:
    tick.wait()
    # time.sleep(1.25)
except KeyboardInterrupt:
    print("Abort wait")
    tick.cancel()

	
print(tick.getEvents())

errMessages=tick.getMessages()
print(errMessages)


playTick=server.play(tick)
playTick.wait()

