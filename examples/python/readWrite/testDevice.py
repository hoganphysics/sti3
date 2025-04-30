import stipy
import stipy.stidevicepy as stidevicepy


class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)

        # *** Define channels *** #

        # Output channels (the device's output actuators)
        self.addOutputChannel(0, stipy.MixedValueType.Double, "coil current")	    #channel 0, must be a double
        self.addOutputChannel(1, stipy.MixedValueType.Int, "temperature setpoint")  #channel 1, must be an integer
        self.addOutputChannel(2, stipy.MixedValueType.Number, "supply voltage")		#channel 2, any numeric type
        self.addOutputChannel(3, stipy.MixedValueType.Vector, "list output")		#channel 3, vector tuple of outputs, format checked by device
        self.addOutputChannel(4, stipy.MixedValueType.String, "string output")

        # Input channel (make measurements that are recorded by the device)
        self.addInputChannel(10, stipy.MixedValueType.Double, "thermocouple voltage")   # measures a double

        # Input/Output channels
        self.addInputChannel(11, stipy.MixedValueType.Number, stipy.MixedValueType.Vector, "vector args")           #measures a number (input); accepts a vector argument (output)
        self.addInputChannel(12, stipy.MixedValueType.Vector, stipy.MixedValueType.Number, "vector measurement")    #measures a vector (input), accepts a number argument (output)

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
        elif channel == 2:
            #supply voltage
            print("Ch:" + str(channel) + ", " + "supply voltage: " + str(value))
            success = True
        elif channel == 3:
            #list output
            print("Ch:" + str(channel) + ", " + "list output: " + str(value))
            mval = stipy.MixedValue()
            mval.setValue(value)
            if mval.isType([stipy.MixedValueType.Number, stipy.MixedValueType.String, stipy.MixedValueType.Boolean]):
			    #do something...
                print("     *vector arg types match!")
            print("     *args: " + str(value))
            success = True
        elif channel == 4:
            #string output
            print("Ch:" + str(channel) + ", " + "string output: " + str(value))
            success = True

        return success
    
    def readChannel(self, channel, value):
        if channel == 10:
            #thermocouple voltage
            print("Read Ch:" + str(channel) + ", " + "in value: " + str(value))
            return 34.5     #double measurement (input)
            success = True
        elif channel == 11:
            #vector arguments (output)
            print("Read ch 11: " + str(value))
            mval = stipy.MixedValue()
            mval.setValue(value)    # wrap in MixedValue to allow type checking, below
            if mval.isType([stipy.MixedValueType.Number, stipy.MixedValueType.String]):
                return 12.2 * value[0].getValue()     #number measurement (input)
                success = True
        elif channel == 12:
            print("Read ch 12: " + str(value))
            return [3.2 * value, "example string result", True]   #vector measurement (input)

        return None

###################################


config = stipy.Configuration(
    {'Device Name': 'TestDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'localhost/0/STI Server'})

device = TestDevice(config)

# Write tests
device.write(0, 12.7)
device.write(1, 32)
device.write(2, 2.5)
device.write(3, [11, "test", False])
device.write(4, "test string")

# Read tests
data = device.read(10)
print("Measurement 1: " + str(data))

data = device.read(11, [12, 'hi'])
print("Measurement 2: " + str(data))

data = device.read(12, 23.4)
print("Measurement 3: " + str(data))


nameServiceAddr = "192.168.1.6:2809"   #OmniORB NameService
hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)

hub.addDevice(device)

hub.run(True)
