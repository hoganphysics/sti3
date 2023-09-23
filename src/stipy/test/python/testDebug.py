from stipy import *
from makeDevMeas import *

did = DeviceID("STI Server", "localhost", 0)
server = connect(did, "192.168.1.4:2809")

s=server.makeshot(f)

tick=server.parse(s)
tick.wait()

print(tick.getParseResult())

result = server.play(tick)
result.wait()

res=result.getShotResult().getMeasurements()[DeviceID("localhost2/0/TestDevice")]
print(res)
