import stipy
import stipy.stidevicepy as stidevicepy


class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)

        # *** Define attributes *** #

        #Simple string attribute
        self.addAttribute("message", "A test message")

        # Float attrribute using class functions for setter/refresher
        self.height = 4.6
        self.addAttribute("Height", str(self.height)).setSetter(self.setHeight).setRefresher(self.refreshHeight)

        # Integer attribute with lambda functions for setter or refresher
        def ds_setter(value):
            ds = int(value)
            if ds > 0:
                self.downsample = ds
                return True     #success
            return False    #illegal value; set failed
        self.addAttribute("Downsample", "1").setSetter(ds_setter).setRefresher(lambda: str(self.downsample))

        #String attribute with list of allowed values
        self.addAttribute("TriggerSource", "Hardware", ["Hardware", "Software"])

        #Attribute with meta data
        def setMode(value):
             print("New mode is " + value)
             return True

        self.addAttribute("Mode", "Mean", ["Mean", "Total"]).setSetter(setMode) \
            .addMetadata("help", "Sets the mode of the device.") \
            .addMetadata("type", "string attribute") #example meta data

        return

    # Example attribute setter/refresher class functions
    def setHeight(self, value):
        newHeight = float(value)

        if (newHeight > 0 and newHeight < 14.7):    #bounds checking
            self.height = newHeight
            return True	    #success
        return False

    def refreshHeight(self):
        return str(self.height)



config = stipy.Configuration(
    {'Device Name': 'TestDevice2',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'localhost/0/STI Server'})

device = TestDevice(config)

nameServiceAddr = "192.168.1.6:2809"   #OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(device)

hub.run(True)



print("message = " + device.getAttribute("message"))
device.setAttribute("message", "Hello")
print("message = " + device.getAttribute("message"))
print("*****")

print("Height = " + device.getAttribute("Height"))
success = device.setAttribute("Height", str(23.6))    #too big, fails
print("success? " + str(success))
print("Height = " + device.getAttribute("Height"))
device.setAttribute("Height", str(5.2))     #ok
print("Height = " + device.getAttribute("Height"))
print("*****")

device.setAttribute("Downsample", str(10))
print("Downsample = " + device.getAttribute("Downsample"))
print("*****")


print("TriggerSource = " + device.getAttribute("TriggerSource"))
device.setAttribute("TriggerSource", "Software")
print("TriggerSource = " + device.getAttribute("TriggerSource"))
print("*****")

at = device.getAttributeManager().getAttribute("Mode")
print(at)
print("Mode = " + at.value())
at.setValue("Total")
print("Mode = " + at.value())
print(dict(at.metadata()))

