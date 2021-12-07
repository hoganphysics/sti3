import stidevicepy
import stipy

from stipy import event
from stipy import ch
from stipy import dev

import time


did=stidevicepy.DeviceID("STI Server", "localhost", 0)
#did=stidevicepy.DeviceID("Digital Out", "localhost", 0)
server=stipy.connect("localhost", did, "192.168.1.2:2809")


def testFunc():
    # print("Test func")
    digital=stipy.STIPyDevice("Digital Out", "localhost", 0, "localhost/0/STI Server")
    deltat = 1*5000000000
    #event(ch(digital, 0), 100, False)
    #event(ch(digital, 0), 140, True)
    #event(ch(digital, 1), 150, True)
    #event(ch(digital, 0), 140 + 1500, False)
#    event(ch(digital, 2), 160, True)
#    event(ch(digital, 2), 260, False)
    #event(ch(digital, 1), 200, False)
    #event(ch(digital, 0), 1000, False)
    #event(ch(digital, 0), 1000+deltat, True)
#    event(ch(digital, 1), 200, False)
    slow=stipy.STIPyDevice("Slow Analog Out", "localhost", 1, "localhost/0/STI Server")
    deltat2 = 1*2000000000
    event(ch(slow, 0), 140, -4.281)
    #event(ch(slow, 0), 140+deltat2, -0.2)
    event(ch(slow, 0), 140+deltat2, 1.2)



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

wait = input("Press Enter to continue.")


