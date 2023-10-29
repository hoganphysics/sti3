import stipy
import stipy.stidevicepy as stidevicepy

import traceback


class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)

        # *** Define channels *** #
        self.addOutputChannel(0, stipy.MixedValueType.Double, "coil current")       #outputs a Double
        self.addOutputChannel(1, stipy.MixedValueType.Number, "supply voltage")     #outputs a number

        # *** Define attributes *** #
        self.addAttribute("x", "22")
        self.addAttribute("TriggerSource", "Hardware", ["Hardware", "Software"])

        return
    
    def writeChannel(self, channel, value):
        success = False

        if channel == 0:
            #coil current (Double)
            print("Ch:" + str(channel) + ", " + "coil current: " + str(value))
            success = True
        elif channel == 1:
            #temperature setpoint (Int)
            print("Ch:" + str(channel) + ", " + "temperature setpoint: " + str(value))
            success = True

        return success

