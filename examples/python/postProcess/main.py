import stipy
import stipy.stidevicepy as stidevicepy

from testDevice import AnalysisDevice


config = stipy.Configuration(
    {'Device Name': 'AnalysisDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'localhost/0/STI Server'})

device = AnalysisDevice(config)

# Discovery: list the post-processing targets the device offers.  This works on a
# local device and on a connected (remote) device reference returned by connect().
for name, description in device.getPostProcessingTargets():
    print("post-processing target:", name, "-", description)

nameServiceAddr = "192.168.1.4:2809"   # OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(device)
hub.run(True)   # pass True to return immediately instead of blocking
