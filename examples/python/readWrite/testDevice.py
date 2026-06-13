import itertools
import random
import struct

import stipy
import stipy.stidevicepy as stidevicepy


class TestDevice(stidevicepy.LocalDevice):
    IMAGE_WIDTH = 10
    IMAGE_HEIGHT = 10
    IMAGE_BYTES_PER_PIXEL = 1
    TIF_IMAGE_WIDTH = 100
    TIF_IMAGE_HEIGHT = 100
    TIF_PANE_COUNT = 3

    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)
        self.image_measurement_index = itertools.count()
        self.file_measurement_index = itertools.count()
        self.virtual_file_measurement_index = itertools.count()

        # *** Define channels *** #

        # Output channels (the device's output actuators)
        ch = self.addOutputChannel(0, stipy.MixedValueType.Double, "coil current")	    #channel 0, must be a double
        ch.setUnits("A")    #set unit to Amperes
        ch.setMinValue(stipy.MixedValue(-10.0))  #set minimum value
        ch.setMaxValue(stipy.MixedValue(10.0))   #set maximum value
        ch.setColor("blue")    #set color for GUI representation
        ch.setValueHint("[-10.0 to 10.0 A]")   #set value hints for GUI representation
        ch.setHelp("Set the coil current. Value must be between -10.0 and 10.0 Amperes.")   #set help string for GUI representation
        
        self.addOutputChannel(1, stipy.MixedValueType.Int, "temperature setpoint")  #channel 1, must be an integer
        self.addOutputChannel(2, stipy.MixedValueType.Number, "supply voltage")		#channel 2, any numeric type
        
        ch = self.addOutputChannel(3, stipy.MixedValueType.Vector, "list output")		#channel 3, vector tuple of outputs, format checked by device
        ch.setVectorFormat([stipy.MixedValueType.Number, stipy.MixedValueType.String, stipy.MixedValueType.Boolean])  #set allowed types for vector elements
        ch.setColor("green")  #set color for GUI representation
        ch.setValueHint("[Frequency (MHz), name, enable]")   #set value hints for GUI representation
        ch.setHelp("Set the list output. Value must be a vector of the form [Frequency (MHz), name, enable].")   #set help string for GUI representation

        self.addOutputChannel(4, stipy.MixedValueType.String, "string output")
        self.addOutputChannel(5, stipy.MixedValueType.Boolean, "enable current")      #channel 5, boolean output

        # Input channel (make measurements that are recorded by the device)
        self.addInputChannel(10, stipy.MixedValueType.Double, "thermocouple voltage")   # measures a double
        self.addInputChannel(13, stipy.MixedValueType.Image, "random image (BinaryData)")   # measures an Image backed by BinaryData
        self.addInputChannel(14, stipy.MixedValueType.Image, "random raw image (FileHolder)")   # measures an Image backed by a raw FileHolder
        self.addInputChannel(15, stipy.MixedValueType.Image, "random multi-pane TIF image (FileHolder)")   # measures an Image backed by a TIF FileHolder
        self.addInputChannel(16, stipy.MixedValueType.File, "example text file (FileHolder)")   # measures a plain text file
        self.addInputChannel(17, stipy.MixedValueType.Binary, "example binary data")   # measures BinaryData that is not an Image
        self.addInputChannel(18, stipy.MixedValueType.File, "example virtual text file (VirtualFileHolder)")   # measures a virtual plain text file

        # Input/Output channels
        ch = self.addInputChannel(11, stipy.MixedValueType.Number, stipy.MixedValueType.Vector, "vector args")           #measures a number (input); accepts a vector argument (output)
        ch.setMeasurementUnits("Hz")
        self.addInputChannel(12, stipy.MixedValueType.Vector, stipy.MixedValueType.Number, "vector measurement")    #measures a vector (input), accepts a number argument (output)

        return

    def random_image_bytes(self, width=None, height=None, bytes_per_pixel=None):
        width = self.IMAGE_WIDTH if width is None else width
        height = self.IMAGE_HEIGHT if height is None else height
        bytes_per_pixel = self.IMAGE_BYTES_PER_PIXEL if bytes_per_pixel is None else bytes_per_pixel
        length = width * height * bytes_per_pixel
        return bytes(random.getrandbits(8) for _ in range(length))

    def random_tif_image_bytes(self):
        panes = [
            self.random_image_bytes(
                self.TIF_IMAGE_WIDTH,
                self.TIF_IMAGE_HEIGHT,
                self.IMAGE_BYTES_PER_PIXEL,
            )
            for _ in range(self.TIF_PANE_COUNT)
        ]
        ifd_entry_count = 12
        ifd_offset = 8
        ifd_size = 2 + (ifd_entry_count * 12) + 4
        image_offset = ifd_offset + (ifd_size * len(panes))

        def ifd_entry(tag, field_type, count, value):
            if field_type == 3:
                if count == 1:
                    value_bytes = struct.pack("<H", value) + b"\x00\x00"
                elif count == 2:
                    value_bytes = struct.pack("<HH", *value)
                else:
                    value_bytes = struct.pack("<I", value)
            else:
                value_bytes = struct.pack("<I", value)
            return struct.pack("<HHI", tag, field_type, count) + value_bytes

        ifds = []
        pane_offset = image_offset
        for pane_index, pixels in enumerate(panes):
            next_ifd_offset = ifd_offset + ifd_size * (pane_index + 1)
            if pane_index == len(panes) - 1:
                next_ifd_offset = 0

            entries = [
                ifd_entry(254, 4, 1, 2),                         # NewSubfileType: one page of a multi-page image
                ifd_entry(256, 4, 1, self.TIF_IMAGE_WIDTH),       # ImageWidth, LONG
                ifd_entry(257, 4, 1, self.TIF_IMAGE_HEIGHT),      # ImageLength, LONG
                ifd_entry(258, 3, 1, 8),                          # BitsPerSample, SHORT
                ifd_entry(259, 3, 1, 1),                          # Compression: none
                ifd_entry(262, 3, 1, 1),                          # PhotometricInterpretation: BlackIsZero
                ifd_entry(273, 4, 1, pane_offset),                # StripOffsets, LONG
                ifd_entry(277, 3, 1, 1),                          # SamplesPerPixel, SHORT
                ifd_entry(278, 4, 1, self.TIF_IMAGE_HEIGHT),      # RowsPerStrip, LONG
                ifd_entry(279, 4, 1, len(pixels)),                # StripByteCounts, LONG
                ifd_entry(284, 3, 1, 1),                          # PlanarConfiguration: chunky
                ifd_entry(297, 3, 2, (pane_index, len(panes))),   # PageNumber, SHORT[2]
            ]

            ifds.append(
                struct.pack("<H", ifd_entry_count)
                + b"".join(entries)
                + struct.pack("<I", next_ifd_offset)
            )
            pane_offset += len(pixels)

        header = struct.pack("<2sHI", b"II", 42, ifd_offset)
        return header + b"".join(ifds) + b"".join(panes)

    def binary_data_backed_image(self):
        return stipy.Image(
            stipy.BinaryData(self.random_image_bytes()),
            self.IMAGE_WIDTH,
            self.IMAGE_HEIGHT,
        )

    def file_holder_backed_image(self, payload, extension, width=None, height=None):
        width = self.IMAGE_WIDTH if width is None else width
        height = self.IMAGE_HEIGHT if height is None else height
        filename = "readWrite-random-image-" + str(next(self.image_measurement_index)) + extension
        persistence = self.getPersistenceManager()
        file_holder = persistence.makeFileHolder(persistence.getTemporaryPath(), filename)

        if file_holder is None or not file_holder.openFile():
            return None

        try:
            if not file_holder.writeBytes(payload):
                return None
        finally:
            file_holder.closeFile()

        image = stipy.Image(file_holder, width, height)
        image.setFileID(file_holder.getID())
        return image

    def raw_file_holder_backed_image(self):
        return self.file_holder_backed_image(self.random_image_bytes(), ".raw")

    def tif_file_holder_backed_image(self):
        return self.file_holder_backed_image(
            self.random_tif_image_bytes(),
            ".tif",
            self.TIF_IMAGE_WIDTH,
            self.TIF_IMAGE_HEIGHT,
        )

    def text_file_holder_measurement(self):
        filename = "readWrite-text-file-" + str(next(self.file_measurement_index)) + ".txt"
        persistence = self.getPersistenceManager()
        file_holder = persistence.makeFileHolder(persistence.getTemporaryPath(), filename)

        if file_holder is None or not file_holder.openFile():
            return None

        try:
            text = (
                "readWrite plain text FileHolder payload\n"
                "This channel returns a MixedValueType.File value, not an Image.\n"
            )
            if not file_holder.writeText(text):
                return None
        finally:
            file_holder.closeFile()

        return file_holder.getID()

    def plain_binary_data_measurement(self):
        payload = (
            b"readWrite plain BinaryData payload\n"
            b"This channel returns BinaryData that is not an Image.\n"
        )
        return stipy.BinaryData(payload)

    def virtual_text_file_measurement(self):
        filename = "readWrite-virtual-text-file-" + str(next(self.virtual_file_measurement_index)) + ".txt"
        persistence = self.getPersistenceManager()
        file_holder = self.makeVirtualFileHolder("readWrite", filename)

        if file_holder is None or not file_holder.openFile():
            return None

        try:
            text = (
                "readWrite virtual text FileHolder payload\n"
                "This channel returns a MixedValueType.File value without writing the source file to disk.\n"
            )
            if not file_holder.writeText(text):
                return None
        finally:
            file_holder.closeFile()

        if not persistence.getFileServer().addFile(file_holder):
            return None
        return file_holder.getID()
    
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
        elif channel == 5:
            #enable current (Boolean)
            print("Ch:" + str(channel) + ", " + "enable current: " + str(value))
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
        elif channel == 13:
            print("Read ch 13: random image (BinaryData)")
            return self.binary_data_backed_image()
        elif channel == 14:
            print("Read ch 14: random raw image (FileHolder)")
            return self.raw_file_holder_backed_image()
        elif channel == 15:
            print("Read ch 15: random TIF image (FileHolder)")
            return self.tif_file_holder_backed_image()
        elif channel == 16:
            print("Read ch 16: example text file (FileHolder)")
            return self.text_file_holder_measurement()
        elif channel == 17:
            print("Read ch 17: example BinaryData")
            return self.plain_binary_data_measurement()
        elif channel == 18:
            print("Read ch 18: example virtual text file (VirtualFileHolder)")
            return self.virtual_text_file_measurement()

        return None

###################################


config = stipy.Configuration(
    {'Device Name': 'TestDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'sr-magis/2/Frame2'})

config.set("NetworkHub", "NameService", "192.168.88.252:2809")

config.set("omniORB", "traceLevel", "0")
# config.set("omniORB", "endPoint", "giop:tcp::2820")
# config.set("omniORB", "endPointPublish", "giop:tcp:192.168.1.109:2820")

config.set("omniORB", "scanGranularity", "1")
config.set("omniORB", "clientConnectTimeOutPeriod", "200")  # milliseconds
config.set("omniORB", "clientCallTimeOutPeriod", "200")  # milliseconds

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

data = device.read(13)
print("Measurement 4: " + str(data))

data = device.read(14)
print("Measurement 5: " + str(data))

data = device.read(15)
print("Measurement 6: " + str(data))

data = device.read(16)
print("Measurement 7: " + str(data))

# Causes segmentation fault in server at startup!
# data = device.read(17)
# print("Measurement 8: " + str(data))

data = device.read(18)
print("Measurement 9: " + str(data))


nameServiceAddr = "192.168.1.242:2809"   #OmniORB NameService
# hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)
hub = stidevicepy.NetworkDeviceHub(config)

hub.addDevice(device)

hub.run(True)
