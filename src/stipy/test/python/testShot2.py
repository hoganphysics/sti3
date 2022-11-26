from stipy import *
from makeMOT import *

did = DeviceID("STI Server", "localhost", 0)
server = connect("localhost", did, "192.168.1.4:2809")

s=server.makeshot(f)

tick=server.parse(s)

tick.wait()

result = server.play(tick)

