import stipy
import stipy.stidevicepy as stidevicepy

from testDevice import AnalysisDevice


config = stipy.Configuration(
    {'Device Name': 'AnalysisDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'sr-magis/2/Frame2'})

device = AnalysisDevice(config)

# Discovery: list the post-processing targets the device offers.  This works on a
# local device and on a connected (remote) device reference returned by connect().
# Each target carries its name, description, and declared option hints.
for target in device.getPostProcessingTargets():
    print("post-processing target:", target.name, "-", target.description)
    for option in target.options:
        print("    option:", option.name, "-", option.description)

nameServiceAddr = "192.168.88.252:2809"   # OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(device)
hub.run(True)   # pass True to return immediately instead of blocking
