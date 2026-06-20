import itertools
import random
import struct
import tempfile
import uuid

import stipy
import stipy.stidevicepy as stidevicepy


class FileTransferDevice(stidevicepy.LocalDevice):
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
        self.last_imported_file_id = None
        self.last_imported_file_size = None

        # Output channel accepting a FileID value imported into this device.
        self.addOutputChannel(6, stipy.MixedValueType.File, "uploaded file (FileID)")

        # File and image-producing input channels.
        self.addInputChannel(13, stipy.MixedValueType.Image, "random image (BinaryData)")
        self.addInputChannel(14, stipy.MixedValueType.Image, "random raw image (FileHolder)")
        self.addInputChannel(15, stipy.MixedValueType.Image, "random multi-pane TIF image (FileHolder)")
        self.addInputChannel(16, stipy.MixedValueType.File, "example text file (FileHolder)")
        self.addInputChannel(17, stipy.MixedValueType.Binary, "example binary data")
        self.addInputChannel(18, stipy.MixedValueType.File, "example virtual text file (VirtualFileHolder)")

        # File and image argument channels.
        self.addInputChannel(19, stipy.MixedValueType.Image, stipy.MixedValueType.Image, "inverted image")
        self.addInputChannel(20, stipy.MixedValueType.Int, stipy.MixedValueType.File, "uploaded file size")

        ch = self.addInputChannel(21, stipy.MixedValueType.Vector, "image vector measurement")
        ch.setVectorFormat([stipy.MixedValueType.Image, stipy.MixedValueType.String, stipy.MixedValueType.Double])

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
                ifd_entry(277, 3, 1, 1),                          # SamplesPerPixel
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

    def invert_image_measurement(self, image):
        if image is None:
            return None

        image_data = image.getData()
        if image_data is None or not image_data.pull():
            print("Read ch 19: input image does not have readable BinaryData")
            return None

        pixels = image_data.getBytes()
        if pixels is None:
            return None

        inverted_pixels = bytes(255 - pixel for pixel in pixels)
        return stipy.Image(inverted_pixels, image.getWidth(), image.getHeight())

    def file_holder_backed_image(self, payload, extension, width=None, height=None):
        width = self.IMAGE_WIDTH if width is None else width
        height = self.IMAGE_HEIGHT if height is None else height
        filename = "fileTransfer-random-image-" + str(next(self.image_measurement_index)) + extension
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
        filename = "fileTransfer-text-file-" + str(next(self.file_measurement_index)) + ".txt"
        persistence = self.getPersistenceManager()
        file_holder = persistence.makeFileHolder(persistence.getTemporaryPath(), filename)

        if file_holder is None or not file_holder.openFile():
            return None

        try:
            text = (
                "fileTransfer plain text FileHolder payload\n"
                "This channel returns a MixedValueType.File value, not an Image.\n"
            )
            if not file_holder.writeText(text):
                return None
        finally:
            file_holder.closeFile()

        return file_holder.getID()

    def plain_binary_data_measurement(self):
        payload = (
            b"fileTransfer plain BinaryData payload\n"
            b"This channel returns BinaryData that is not an Image.\n"
        )
        return stipy.BinaryData(payload)

    def virtual_text_file_measurement(self):
        filename = "fileTransfer-virtual-text-file-" + str(next(self.virtual_file_measurement_index)) + ".txt"
        persistence = self.getPersistenceManager()
        file_holder = self.makeVirtualFileHolder("fileTransfer", filename)

        if file_holder is None or not file_holder.openFile():
            return None

        try:
            text = (
                "fileTransfer virtual text FileHolder payload\n"
                "This channel returns a MixedValueType.File value without writing the source file to disk.\n"
            )
            if not file_holder.writeText(text):
                return None
        finally:
            file_holder.closeFile()

        if not persistence.getFileServer().addFile(file_holder):
            return None
        return file_holder.getID()

    def imported_file_size(self, file_id):
        if file_id is None:
            return None

        persistence = self.getPersistenceManager()
        file_server = persistence.getFileServer()
        if file_server is None or not file_server.findFile(file_id):
            return None

        return file_server.getFileSize(file_id)

    def image_vector_measurement(self):
        return [self.binary_data_backed_image(), "image vector result", 2.5]

    def writeChannel(self, channel, value):
        if channel == 6:
            file_size = self.imported_file_size(value)
            if file_size is not None:
                self.last_imported_file_id = value
                self.last_imported_file_size = file_size
                print("Ch:" + str(channel) + ", uploaded file: " + str(file_size) + " bytes")
                return True

            print("Ch:" + str(channel) + ", uploaded file is not available: " + str(value))

        return False

    def readChannel(self, channel, value):
        if channel == 13:
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
        elif channel == 19:
            print("Read ch 19: invert image")
            return self.invert_image_measurement(value)
        elif channel == 20:
            print("Read ch 20: uploaded file size")
            return self.imported_file_size(value)
        elif channel == 21:
            print("Read ch 21: image vector measurement")
            return self.image_vector_measurement()

        return None


def make_import_source(persistence):
    payload = (
        b"fileTransfer FileID import example\n"
        b"The target device should receive this through PersistenceManager.importFile().\n"
    )
    source_server = persistence.makeVirtualFileServer()
    source_holder = persistence.makeFileHolder(
        tempfile.gettempdir(),
        "fileTransfer-import-source-" + uuid.uuid4().hex + ".txt",
    )

    if source_server is None or source_holder is None or not source_holder.openFile():
        return None, None, payload

    try:
        if not source_holder.writeBytes(payload):
            return None, None, payload
    finally:
        source_holder.closeFile()

    if not source_server.addFile(source_holder):
        return None, None, payload

    return source_holder, source_server, payload


config = stipy.Configuration(
    {'Device Name': 'FileTransferDevice',
     'IP Address': 'localhost',
     'Module': '0',
     'Target Server': 'sr-magis/2/Frame2'})

config.set("NetworkHub", "NameService", "192.168.88.252:2809")

config.set("omniORB", "traceLevel", "0")
config.set("omniORB", "scanGranularity", "1")
config.set("omniORB", "clientConnectTimeOutPeriod", "200")  # milliseconds
config.set("omniORB", "clientCallTimeOutPeriod", "200")  # milliseconds

device = FileTransferDevice(config)

# Read tests
print("Measurement 13: " + str(device.read(13)))
print("Measurement 14: " + str(device.read(14)))
print("Measurement 15: " + str(device.read(15)))
print("Measurement 16: " + str(device.read(16)))
print("Measurement 17: " + str(device.read(17)))
print("Measurement 18: " + str(device.read(18)))

input_image = stipy.Image(device.random_image_bytes(), device.IMAGE_WIDTH, device.IMAGE_HEIGHT)
print("Measurement 19: " + str(device.read(19, input_image)))
print("Measurement 21: " + str(device.read(21)))

persistence = device.getPersistenceManager()
source_holder, source_server, payload = make_import_source(persistence)
if source_holder is not None and source_server is not None:
    with persistence.importFile(source_holder.getID(), source_server) as imported:
        device.write(6, imported.fileID)
        print("Measurement 20: " + str(device.read(20, imported.fileID)))


# nameServiceAddr = "192.168.1.242:2809"   #OmniORB NameService
# hub = stidevicepy.NetworkDeviceHub(nameServiceAddr)
hub = stidevicepy.NetworkDeviceHub(config)

hub.addDevice(device)

hub.run(True)
