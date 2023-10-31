import stipy
import stipy.stidevicepy as stidevicepy

import traceback


class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)

        # *** Define channels *** #
        self.addOutputChannel(0, stipy.MixedValueType.Double, "coil current")       #outputs a Double
        self.addOutputChannel(1, stipy.MixedValueType.Number, "supply voltage")     #outputs a number
        self.addInputChannel(11, stipy.MixedValueType.Number, stipy.MixedValueType.Vector, "vector args")  #measures a number (input); accepts a vector argument (output)

        # *** Define attributes *** #
        self.addAttribute("x", "22")
        self.addAttribute("TriggerSource", "Hardware", ["Hardware", "Software"])


        # *** Logging examples *** #

        self.log().append("Constructing TestDevice")        #write to default log

        self.log("testing").append("A log comment")         #write to a custom log named "testing"

        # Examples of scheduling regularly repeating log tasks:
        # The time interval between tasks is a string with format hh:mm:ss

        # Every 5 seconds, log the value of attribute 'x'
        self.log().addAttributeLogTask("x", "00:00:05")

        # Every 6 seconds, set channel 0 to 11.2 and log the result
        self.log().addWriteLogTask(0, "00:00:06", 11.2)

        # Every 2 seconds, read channel 11 and log the result
        self.log("testing").addReadLogTask(11, "00:00:02", [5.7, "example data"])

        # Log tasks can also use a custom function to generate the values used by read and write tasks.

        self.testValue = 0.5    # Member variable used by log task generating functions, below
       
        # Every 4 seconds, write to channel 0 with a custom value generating function, and log the result
        self.log("testing").addWriteLogTask(0, "00:00:03", self.get_test_value)

        # Another example of a value generating function, used by log task below
        def get_vec_value():
            print("get_vec_value: ", self.testValue)
            self.testValue += 1
            return [self.testValue, "more example data"]    # this will be used as the value argument for read(...) in the task below

        # Every 5 seconds, read ch 11 using generating function and log the result
        self.log("testing").addReadLogTask(11, "00:00:05", get_vec_value)

        return


    def get_test_value(self):
        print("get_test_value: " + str(self.testValue))
        self.testValue += 1
        return self.testValue   # this will be used as the value argument for write(...) in the task defined above
    
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

    def readChannel(self, channel, value):
        if channel == 11:
            #vector arguments (output)
            mval = stipy.MixedValue()
            mval.setValue(value)    # wrap in MixedValue to allow type checking, below
            if mval.isType([stipy.MixedValueType.Number, stipy.MixedValueType.String]):
                return 12.2 * value[0]     #number measurement (input)
                success = True
        return None

