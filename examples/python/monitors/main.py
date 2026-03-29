import stipy
import stipy.stidevicepy as stidevicepy

from testDevice import TestDevice


config = stipy.Configuration(
    {'Device Name': 'MonitorExampleDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'sr-magis/2/Frame2'})

device = TestDevice(config)

task_manager = device.getTaskManager()
for task_id in task_manager.getTaskIDs():
    task_manager.runTask(task_id)

device.advanceState()

monitor_manager = device.getMonitorManager()
print("Monitor IDs:", monitor_manager.getIDs())
print("Temperature:", monitor_manager.getValue("Status/temperatureC"))
print("State:", monitor_manager.getValue("Status/state"))

nameServiceAddr = "192.168.1.109:2809"   # OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(device)
hub.run(True)
