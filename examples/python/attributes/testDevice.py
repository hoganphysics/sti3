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
        self.downsample = 1
        self.addAttribute("Downsample", "1").setSetter(ds_setter).setRefresher(lambda: str(self.downsample))

        #String attribute with list of allowed values
        #This attribute has an attibute group specified with a prefix of "Trigger::" 
        self.addAttribute("Trigger::TriggerSource", "Hardware", ["Hardware", "Software"])

        #Attribute with meta data
        def setMode(value):
             print("New mode is " + value)
             return True

        self.addAttribute("Mode", "Mean", ["Mean", "Total"]).setSetter(setMode) \
            .addMetadata("help", "Sets the mode of the device.") \
            .addMetadata("type", "string attribute") #example meta data
        
        # self.addAttribute("Enable Trigger", "True", ["True", "False"])
        #Another attribute in the same group as the TriggerSource attribute 
        self.addAttribute("Trigger::Enable Trigger", "On", ["On", "Off"])

        # These attributes describe one hardware region. Changing either
        # dimension also changes the derived pixel count.
        self.region_width = 640
        self.region_height = 480

        def set_region_width(value):
            width = int(value)
            if width <= 0:
                return False
            self.region_width = width
            return True

        def set_region_height(value):
            height = int(value)
            if height <= 0:
                return False
            self.region_height = height
            return True

        self.addAttribute("Region::Width", str(self.region_width)) \
            .setSetter(set_region_width) \
            .setRefresher(lambda: str(self.region_width))
        self.addAttribute("Region::Height", str(self.region_height)) \
            .setSetter(set_region_height) \
            .setRefresher(lambda: str(self.region_height))
        self.addAttribute("Region::PixelCount", str(self.region_width * self.region_height)) \
            .setSetter(lambda value: False) \
            .setRefresher(lambda: str(self.region_width * self.region_height))

        self.addAttributeRefreshGroup([
            "Region::Width",
            "Region::Height",
            "Region::PixelCount",
        ])

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
     'Target Server': 'sr-magis/2/Frame2'})

config.set("NetworkHub", "NameService", "192.168.88.252:2809")

device = TestDevice(config)

# nameServiceAddr = "192.168.22.254:2809"   #OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(config)

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
device.height = 6.2       #changed outside setAttribute()
device.refreshAttribute("Height")
print("Height = " + device.getAttribute("Height"))
print("*****")

device.setAttribute("Downsample", str(10))
print("Downsample = " + device.getAttribute("Downsample"))
device.downsample = 12    #changed outside setAttribute()
device.getAttributeManager().refreshValue("Downsample")
print("Downsample = " + device.getAttribute("Downsample"))
device.height = 7.1
device.downsample = 13
device.refreshAttributes()
print("Height = " + device.getAttribute("Height"))
print("Downsample = " + device.getAttribute("Downsample"))
print("*****")


print("TriggerSource = " + device.getAttribute("Trigger::TriggerSource"))
device.setAttribute("Trigger::TriggerSource", "Software")
print("TriggerSource = " + device.getAttribute("Trigger::TriggerSource"))
print("*****")

# Setting Width refreshes all three group members after the setter completes.
device.setAttribute("Region::Width", "800")
print("Region = "
      + device.getAttribute("Region::Width") + " x "
      + device.getAttribute("Region::Height"))
print("PixelCount = " + device.getAttribute("Region::PixelCount"))

# A hardware-side change can be synchronized through any group member.
device.region_width = 1024
device.region_height = 768
device.refreshAttribute("Region::PixelCount")
print("Refreshed Region = "
      + device.getAttribute("Region::Width") + " x "
      + device.getAttribute("Region::Height"))
print("PixelCount = " + device.getAttribute("Region::PixelCount"))
print("*****")

at = device.getAttributeManager().getAttribute("Mode")
print(at)
print("Mode = " + at.value())
at.setValue("Total")
print("Mode = " + at.value())
print(dict(at.metadata()))

