import stidevicepy
import stipy

from stipy import event
from stipy import ch
from stipy import dev

import time

ns=1
us=1000*ns
ms=1000*us
s=1000*ms

#hubid=stipy.HubID("Digital Out", "localhost", 0)
did=stidevicepy.DeviceID("STI Server", "localhost", 0)
#did=stidevicepy.DeviceID("Slow Analog Out", "localhost", 1)
server=stipy.connect("localhost", did,  "192.168.1.2:2809")


def testFunc():
    print("Test func")
    digital=stipy.STIPyDevice("Digital Out", "localhost", 0, "localhost/0/STI Server")
    time = 100000
    ts=100*us
    event(ch(digital, 0), time + ts, True)
    event(ch(digital, 0), time+40+ ts, False)
    event(ch(digital, 0), time+80+ ts, True)
	
    #digital=stipy.STIPyDevice("Digital Out", "localhost", 0, "localhost/0/STI Server")
    slow=stipy.STIPyDevice("Slow Analog Out", "localhost", 1, "localhost/0/STI Server")
    #deltat = 1*5000000000
    deltat = 1*2000000000
    event(ch(slow, 4), time, 3.2)
    event(ch(slow, 4), time+(deltat/2), 0.0)
    event(ch(slow, 4), time+deltat, -3.2)
    
#    event(ch(digital, 1), 200, False)


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


