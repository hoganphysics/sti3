"""stidevicepy runtime coverage for lazy binary channel measurements."""

import os
from pathlib import Path
import subprocess
import struct
import time
import uuid

import pytest

from sti_testnet.devices import ChannelSpec
from sti_testnet.devices import DeviceSpec
from sti_testnet.devices import make_server_spec
from sti_testnet.topology import InProcessTopology
from sti_testnet.waits import wait_for
from sti_testnet.waits import wait_for_device_ids
from sti_testnet.waits import wait_for_ticket


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice]


def _repo_root():
    return Path(__file__).resolve().parents[3]


def _build_dir():
    return Path(os.environ.get("STI3_BUILD_DIR", _repo_root() / "build-ninja")).resolve()


def _stiserver_binary():
    binary = _build_dir() / "src" / "server" / "src" / "STIServer"
    if not binary.exists():
        pytest.skip("STIServer binary is not built: {0}".format(binary))
    return binary


def _server_config(tmp_path, nameservice_address, name, address, module):
    config_path = tmp_path / "stiserver.ini"
    config_path.write_text(
        "\n".join(
            [
                "Device Name = {0}".format(name),
                "IP Address = {0}".format(address),
                "Module = {0}".format(module),
                "Target Server = root",
                "EnableActivate = true",
                "EnableDeactivate = true",
                "",
                "[NetworkHub]",
                "NameService = {0}".format(nameservice_address),
                "",
                "[omniORB]",
                "traceLevel = 0",
                "",
                "[Shot Repository]",
                "Path = {0}".format(tmp_path / "server-shots"),
                "",
            ]
        )
    )
    return config_path


class StiServerProcess(object):
    def __init__(self, binary, config_path, nameservice_address):
        self.binary = binary
        self.config_path = config_path
        self.nameservice_address = nameservice_address
        self.process = None
        self.stdout = ""
        self.stderr = ""

    def start(self):
        self.process = subprocess.Popen(
            [
                str(self.binary),
                "--file",
                str(self.config_path),
                "--NameService",
                self.nameservice_address,
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            close_fds=True,
        )
        return self

    def shutdown(self):
        process = self.process
        self.process = None
        if process is None:
            return
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=2.0)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=2.0)
        self._collect_output(process)

    def assert_running(self):
        if self.process is not None and self.process.poll() is not None:
            self._collect_output(self.process)
            raise AssertionError("STIServer exited unexpectedly\n{0}".format(self.diagnostics()))
        return True

    def diagnostics(self):
        return "\n".join(
            [
                "STIServer binary: {0}".format(self.binary),
                "STIServer config: {0}".format(self.config_path),
                "STIServer returncode: {0}".format(None if self.process is None else self.process.poll()),
                "stdout:\n{0}".format(self.stdout),
                "stderr:\n{0}".format(self.stderr),
            ]
        )

    def _collect_output(self, process):
        try:
            stdout, stderr = process.communicate(timeout=0.1)
        except subprocess.TimeoutExpired:
            return
        self.stdout += stdout or ""
        self.stderr += stderr or ""


class BinaryMeasurementEvent(object):
    def __new__(cls, stidevicepy, event_time, raw_events, payload):
        class _BinaryMeasurementEvent(stidevicepy.SynchronousEvent):
            def __init__(self):
                stidevicepy.SynchronousEvent.__init__(self, event_time)
                self.payload = payload
                for event in raw_events:
                    self.addMeasurement(event)

            def loadEvent(self):
                return

            def playEvent(self):
                return

            def collectMeasurementData(self):
                import stipy

                binary = stipy.BinaryData(self.payload)
                for measurement in self.getMeasurements():
                    measurement.setMeasurementResult(binary)

        return _BinaryMeasurementEvent()


class BinaryMeasurementDevice(object):
    def __new__(cls, stidevicepy, spec, payload):
        class _BinaryMeasurementDevice(stidevicepy.LocalDevice):
            def __init__(self):
                self.spec = spec
                self.payload = payload
                stidevicepy.LocalDevice.__init__(self, spec.config())
                self.addInputChannel(0, spec.input_channels[0].value_type, spec.input_channels[0].name)

            def parseEvents(self, eventsIn, synchedEvents):
                for event_time, events in list(eventsIn.items()):
                    measurement_events = [event for event in events if event.isMeasurementEvent()]
                    if measurement_events:
                        synchedEvents.append(
                            BinaryMeasurementEvent(stidevicepy, event_time, measurement_events, self.payload)
                        )

        return _BinaryMeasurementDevice()


class LazyReadDevice(object):
    def __new__(cls, stidevicepy, spec, payload):
        class _LazyReadDevice(stidevicepy.LocalDevice):
            def __init__(self):
                self.spec = spec
                self.payload = payload
                stidevicepy.LocalDevice.__init__(self, spec.config())
                for channel in spec.input_channels:
                    self.addInputChannel(channel.number, channel.value_type, channel.name)

            def readChannel(self, channel, value):
                import stipy

                binary = stipy.BinaryData(self.payload)
                if channel == 0:
                    return binary
                if channel == 1:
                    return stipy.Image(binary, width=10, height=10)
                return None

        return _LazyReadDevice()


class ReadWriteBinaryChannelDevice(object):
    def __new__(cls, stidevicepy, spec, payload):
        class _ReadWriteBinaryChannelDevice(stidevicepy.LocalDevice):
            def __init__(self):
                self.spec = spec
                self.payload = payload
                stidevicepy.LocalDevice.__init__(self, spec.config())
                channel = spec.input_channels[0]
                self.addInputChannel(channel.number, channel.value_type, channel.name)

            def readChannel(self, channel, value):
                import stipy

                if channel == 17:
                    return stipy.BinaryData(self.payload)
                return None

        return _ReadWriteBinaryChannelDevice()


class FileBackedImageReadDevice(object):
    IMAGE_WIDTH = 10
    IMAGE_HEIGHT = 10

    def __new__(cls, stidevicepy, spec):
        class _FileBackedImageReadDevice(stidevicepy.LocalDevice):
            IMAGE_WIDTH = cls.IMAGE_WIDTH
            IMAGE_HEIGHT = cls.IMAGE_HEIGHT

            def __init__(self):
                self.spec = spec
                self.image_measurement_index = 0
                stidevicepy.LocalDevice.__init__(self, spec.config())
                for channel in spec.input_channels:
                    self.addInputChannel(channel.number, channel.value_type, channel.name)

            def raw_image_bytes(self):
                length = self.IMAGE_WIDTH * self.IMAGE_HEIGHT
                return bytes((index % 256 for index in range(length)))

            def tif_image_bytes(self):
                pixels = self.raw_image_bytes()
                ifd_entry_count = 10
                ifd_offset = 8
                image_offset = ifd_offset + 2 + (ifd_entry_count * 12) + 4

                def ifd_entry(tag, field_type, count, value):
                    if field_type == 3 and count == 1:
                        value_bytes = struct.pack("<H", value) + b"\x00\x00"
                    else:
                        value_bytes = struct.pack("<I", value)
                    return struct.pack("<HHI", tag, field_type, count) + value_bytes

                entries = [
                    ifd_entry(256, 4, 1, self.IMAGE_WIDTH),
                    ifd_entry(257, 4, 1, self.IMAGE_HEIGHT),
                    ifd_entry(258, 3, 1, 8),
                    ifd_entry(259, 3, 1, 1),
                    ifd_entry(262, 3, 1, 1),
                    ifd_entry(273, 4, 1, image_offset),
                    ifd_entry(277, 3, 1, 1),
                    ifd_entry(278, 4, 1, self.IMAGE_HEIGHT),
                    ifd_entry(279, 4, 1, len(pixels)),
                    ifd_entry(284, 3, 1, 1),
                ]

                header = struct.pack("<2sHI", b"II", 42, ifd_offset)
                ifd = struct.pack("<H", ifd_entry_count) + b"".join(entries) + struct.pack("<I", 0)
                return header + ifd + pixels

            def file_holder_backed_image(self, payload, extension):
                import stipy

                self.image_measurement_index += 1
                filename = "readWrite-random-image-{0}{1}".format(self.image_measurement_index, extension)
                persistence = self.getPersistenceManager()
                file_holder = persistence.makeFileHolder(persistence.getTemporaryPath(), filename)

                if file_holder is None or not file_holder.openFile():
                    return None

                try:
                    if not file_holder.writeBytes(payload):
                        return None
                finally:
                    file_holder.closeFile()

                image = stipy.Image(file_holder, self.IMAGE_WIDTH, self.IMAGE_HEIGHT)
                image.setFileID(file_holder.getID())
                return image

            def readChannel(self, channel, value):
                if channel == 14:
                    return self.file_holder_backed_image(self.raw_image_bytes(), ".raw")
                if channel == 15:
                    return self.file_holder_backed_image(self.tif_image_bytes(), ".tif")
                return None

        return _FileBackedImageReadDevice()


class FileTransferSequenceDevice(object):
    IMAGE_WIDTH = 10
    IMAGE_HEIGHT = 10
    IMAGE_BYTES_PER_PIXEL = 1
    TIF_IMAGE_WIDTH = 100
    TIF_IMAGE_HEIGHT = 100
    TIF_PANE_COUNT = 3

    def __new__(cls, stidevicepy, spec):
        class _FileTransferSequenceDevice(stidevicepy.LocalDevice):
            IMAGE_WIDTH = cls.IMAGE_WIDTH
            IMAGE_HEIGHT = cls.IMAGE_HEIGHT
            IMAGE_BYTES_PER_PIXEL = cls.IMAGE_BYTES_PER_PIXEL
            TIF_IMAGE_WIDTH = cls.TIF_IMAGE_WIDTH
            TIF_IMAGE_HEIGHT = cls.TIF_IMAGE_HEIGHT
            TIF_PANE_COUNT = cls.TIF_PANE_COUNT

            def __init__(self):
                import stipy

                self.spec = spec
                self.image_measurement_index = 0
                self.file_measurement_index = 0
                self.virtual_file_measurement_index = 0
                stidevicepy.LocalDevice.__init__(self, spec.config())

                self.addOutputChannel(6, spec.output_channels[0].value_type, spec.output_channels[0].name)
                for channel in spec.input_channels:
                    if channel.number == 19:
                        self.addInputChannel(
                            channel.number,
                            channel.value_type,
                            stipy.MixedValueType.Image,
                            channel.name,
                        )
                    elif channel.number == 20:
                        self.addInputChannel(
                            channel.number,
                            channel.value_type,
                            stipy.MixedValueType.File,
                            channel.name,
                        )
                    elif channel.number == 21:
                        ch = self.addInputChannel(channel.number, channel.value_type, channel.name)
                        ch.setVectorFormat(
                            [
                                stipy.MixedValueType.Image,
                                stipy.MixedValueType.String,
                                stipy.MixedValueType.Double,
                            ]
                        )
                    else:
                        self.addInputChannel(channel.number, channel.value_type, channel.name)

            def random_image_bytes(self, width=None, height=None, bytes_per_pixel=None):
                width = self.IMAGE_WIDTH if width is None else width
                height = self.IMAGE_HEIGHT if height is None else height
                bytes_per_pixel = self.IMAGE_BYTES_PER_PIXEL if bytes_per_pixel is None else bytes_per_pixel
                length = width * height * bytes_per_pixel
                return bytes((index % 256 for index in range(length)))

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
                        ifd_entry(254, 4, 1, 2),
                        ifd_entry(256, 4, 1, self.TIF_IMAGE_WIDTH),
                        ifd_entry(257, 4, 1, self.TIF_IMAGE_HEIGHT),
                        ifd_entry(258, 3, 1, 8),
                        ifd_entry(259, 3, 1, 1),
                        ifd_entry(262, 3, 1, 1),
                        ifd_entry(273, 4, 1, pane_offset),
                        ifd_entry(277, 3, 1, 1),
                        ifd_entry(278, 4, 1, self.TIF_IMAGE_HEIGHT),
                        ifd_entry(279, 4, 1, len(pixels)),
                        ifd_entry(284, 3, 1, 1),
                        ifd_entry(297, 3, 2, (pane_index, len(panes))),
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
                import stipy

                return stipy.Image(
                    stipy.BinaryData(self.random_image_bytes()),
                    self.IMAGE_WIDTH,
                    self.IMAGE_HEIGHT,
                )

            def file_holder_backed_image(self, payload, extension, width=None, height=None):
                import stipy

                width = self.IMAGE_WIDTH if width is None else width
                height = self.IMAGE_HEIGHT if height is None else height
                self.image_measurement_index += 1
                filename = "fileTransfer-random-image-{0}{1}".format(self.image_measurement_index, extension)
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

            def text_file_holder_measurement(self):
                self.file_measurement_index += 1
                filename = "fileTransfer-text-file-{0}.txt".format(self.file_measurement_index)
                persistence = self.getPersistenceManager()
                file_holder = persistence.makeFileHolder(persistence.getTemporaryPath(), filename)

                if file_holder is None or not file_holder.openFile():
                    return None

                try:
                    payload = (
                        b"fileTransfer plain text FileHolder payload\n"
                        b"This channel returns a MixedValueType.File value, not an Image.\n"
                    )
                    if not file_holder.writeBytes(payload):
                        return None
                finally:
                    file_holder.closeFile()

                return file_holder.getID()

            def plain_binary_data_measurement(self):
                import stipy

                payload = (
                    b"fileTransfer plain BinaryData payload\n"
                    b"This channel returns BinaryData that is not an Image.\n"
                )
                return stipy.BinaryData(payload)

            def virtual_text_file_measurement(self):
                self.virtual_file_measurement_index += 1
                filename = "fileTransfer-virtual-text-file-{0}.txt".format(self.virtual_file_measurement_index)
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

            def invert_image_measurement(self, image):
                import stipy

                if image is None:
                    return None

                image_data = image.getData()
                if image_data is None or not image_data.pull():
                    return None

                pixels = image_data.getBytes()
                if pixels is None:
                    return None

                inverted_pixels = bytes(255 - pixel for pixel in pixels)
                return stipy.Image(inverted_pixels, image.getWidth(), image.getHeight())

            def readChannel(self, channel, value):
                if channel == 13:
                    return self.binary_data_backed_image()
                if channel == 14:
                    return self.file_holder_backed_image(self.random_image_bytes(), ".raw")
                if channel == 15:
                    return self.file_holder_backed_image(
                        self.random_tif_image_bytes(),
                        ".tif",
                        self.TIF_IMAGE_WIDTH,
                        self.TIF_IMAGE_HEIGHT,
                    )
                if channel == 16:
                    return self.text_file_holder_measurement()
                if channel == 17:
                    return self.plain_binary_data_measurement()
                if channel == 18:
                    return self.virtual_text_file_measurement()
                if channel == 19:
                    return self.invert_image_measurement(value)
                if channel == 20:
                    return self.imported_file_size(value)
                if channel == 21:
                    return [self.binary_data_backed_image(), "image vector result", 2.5]
                return None

            def writeChannel(self, channel, value):
                if channel == 6:
                    return self.imported_file_size(value) is not None
                return False

        return _FileTransferSequenceDevice()


def last_image(remote_device, channel_number, stipy):
    channel = remote_device.getChannelManager().getChannel(channel_number)
    measurement = channel.getLastMeasurement()
    assert measurement.getType() == stipy.MixedValueType.Image
    return measurement.getImage()


def save_image_file_to_path(stipy, remote_device, image, output_path):
    persistence = remote_device.getPersistenceManager()
    assert persistence is not None

    file_server = persistence.getFileServer()
    assert file_server is not None

    file_id = image.getFileID()
    destination = persistence.makeFileHolder(str(output_path.parent), output_path.name)
    assert destination is not None

    assert file_server.transferFile(file_id, destination, stipy.FileTransferType.Binary)
    assert destination.exists()


def receive_image_file_to_virtual_server(stipy, remote_device, image):
    persistence = remote_device.getPersistenceManager()
    assert persistence is not None

    file_server = persistence.getFileServer()
    assert file_server is not None

    virtual_file_server = persistence.makeVirtualFileServer()
    assert virtual_file_server is not None

    source_file_id = image.getFileID()
    backing_destination = stipy.VirtualFileHolder("stipy-virtual-receiver", source_file_id)
    destination = persistence.makeVirtualFileHolder(backing_destination)
    assert destination is not None

    assert file_server.transferFile(source_file_id, destination, stipy.FileTransferType.Binary)

    assert virtual_file_server.addFile(backing_destination)
    received_file_id = backing_destination.getID()
    received_bytes = backing_destination.getBytes()

    assert virtual_file_server.findFile(received_file_id)
    assert virtual_file_server.getFileSize(received_file_id) == len(received_bytes)

    roundtrip_destination = stipy.VirtualFileHolder("stipy-virtual-roundtrip", received_file_id)
    assert virtual_file_server.transferFile(
        received_file_id,
        roundtrip_destination,
        stipy.FileTransferType.Binary,
    )

    return received_bytes, roundtrip_destination.getBytes()


def make_import_source_for_test(persistence, tmp_path):
    payload = (
        b"fileTransfer FileID import example\n"
        b"The target device should receive this through PersistenceManager.importFile().\n"
    )
    source_server = persistence.makeVirtualFileServer()
    source_holder = persistence.makeFileHolder(
        str(tmp_path),
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


def test_stidevicepy_can_explicitly_pull_lazy_binary_channel_measurement(
    sti_nameservice_address,
    stipy_modules,
    tmp_path,
):
    stipy, stidevicepy = stipy_modules

    payload = b"lazy-channel-binary-payload\x00with-nul"
    server_spec = make_server_spec(name="stidevicepy Binary Server", address="localhost", module=50)
    server_id = server_spec.device_id()
    device_spec = DeviceSpec(
        name="stidevicepy Binary Device",
        address="localhost",
        module=51,
        target_server_id=server_id.getID(),
        output_channels=[],
        input_channels=[
            ChannelSpec(0, name="binary measurement", value_type=stipy.MixedValueType.Binary, direction="input")
        ],
    )

    with InProcessTopology(sti_nameservice_address, server_spec, []) as topology:
        device = BinaryMeasurementDevice(stidevicepy, device_spec, payload)
        topology.devices.append(device)
        topology.hub.addDevice(device)

        wait_for_device_ids(
            topology.hub,
            [server_id, device_spec.device_id()],
            timeout_s=5.0,
            diagnostics=topology.diagnostics,
        )

        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        def binary_measurement_shot():
            stipy.meas(stipy.ch(stipy.dev(device_spec.device_id()), 0), 1000)

        shot = server.makeshot(binary_measurement_shot)
        parse_ticket = server.parse(shot)
        wait_for_ticket(parse_ticket, timeout_s=10.0, diagnostics=topology.diagnostics)

        result_ticket = server.play(parse_ticket)
        wait_for_ticket(result_ticket, timeout_s=10.0, diagnostics=topology.diagnostics)

        remote_device = server.getDeviceCollection().get(device_spec.device_id())
        remote_channel = remote_device.getChannelManager().getChannel(0)
        measurement = remote_channel.getLastMeasurement()

        assert measurement.getType() == stipy.MixedValueType.Binary
        binary = measurement.getBinary()
        assert binary is not None
        assert binary.bytes() == len(payload)
        assert binary.wordsize() == 1
        assert binary.hasStream()
        assert not binary.hasLocalData()
        assert not binary.isMaterialized()

        assert binary.pull()
        assert binary.isMaterialized()
        assert binary.hasLocalData()
        assert binary.getBytes() == payload
        assert measurement.getValue() == payload

        output_path = tmp_path / "measurement.bin"
        assert binary.save(str(output_path))
        assert output_path.read_bytes() == payload


def test_stidevicepy_read_returns_lazy_binary_and_image_payloads(
    sti_nameservice_address,
    stipy_modules,
):
    stipy, stidevicepy = stipy_modules

    payload = b"lazy-read-binary-payload\x00with-nul"
    server_spec = make_server_spec(name="stidevicepy Lazy Read Server", address="localhost", module=52)
    server_id = server_spec.device_id()
    device_spec = DeviceSpec(
        name="stidevicepy Lazy Read Device",
        address="localhost",
        module=53,
        target_server_id=server_id.getID(),
        output_channels=[],
        input_channels=[
            ChannelSpec(0, name="binary read", value_type=stipy.MixedValueType.Binary, direction="input"),
            ChannelSpec(1, name="image read", value_type=stipy.MixedValueType.Image, direction="input"),
        ],
    )

    with InProcessTopology(sti_nameservice_address, server_spec, []) as topology:
        device = LazyReadDevice(stidevicepy, device_spec, payload)
        topology.devices.append(device)
        topology.hub.addDevice(device)

        wait_for_device_ids(
            topology.hub,
            [server_id, device_spec.device_id()],
            timeout_s=5.0,
            diagnostics=topology.diagnostics,
        )

        server = topology.connect_stipy()
        assert server is not None, topology.summary()
        remote_device = server.getDeviceCollection().get(device_spec.device_id())

        binary = remote_device.read(0)
        assert isinstance(binary, stipy.BinaryData)
        assert binary.bytes() == len(payload)
        assert binary.wordsize() == 1
        assert binary.hasStream()
        assert not binary.hasLocalData()
        assert not binary.isMaterialized()

        assert binary.pull()
        assert binary.hasLocalData()
        assert binary.getBytes() == payload

        image = remote_device.read(1)
        assert isinstance(image, stipy.Image)
        assert image.getWidth() == 10
        assert image.getHeight() == 10
        assert image.hasData()

        image_data = image.getData()
        assert image_data.bytes() == len(payload)
        assert image_data.hasStream()
        assert not image_data.hasLocalData()
        assert not image_data.isMaterialized()

        assert image_data.pull()
        assert image_data.hasLocalData()
        assert image_data.getBytes() == payload


def test_stidevicepy_readwrite_channel_17_binary_read_does_not_crash_server(
    sti_nameservice_address,
    stipy_modules,
):
    stipy, stidevicepy = stipy_modules

    payload = (
        b"readWrite plain BinaryData payload\n"
        b"This channel returns BinaryData that is not an Image.\n"
    )
    server_spec = make_server_spec(name="stidevicepy ReadWrite Binary Server", address="localhost", module=54)
    server_id = server_spec.device_id()
    device_spec = DeviceSpec(
        name="TestDevice",
        address="localhost",
        module=0,
        target_server_id=server_id.getID(),
        output_channels=[],
        input_channels=[
            ChannelSpec(17, name="example binary data", value_type=stipy.MixedValueType.Binary, direction="input"),
        ],
    )

    with InProcessTopology(sti_nameservice_address, server_spec, []) as topology:
        device = ReadWriteBinaryChannelDevice(stidevicepy, device_spec, payload)
        topology.devices.append(device)
        topology.hub.addDevice(device)

        wait_for_device_ids(
            topology.hub,
            [server_id, device_spec.device_id()],
            timeout_s=5.0,
            diagnostics=topology.diagnostics,
        )

        server = topology.connect_stipy()
        assert server is not None, topology.summary()
        remote_device = server.getDeviceCollection().get(device_spec.device_id())

        binary = remote_device.read(17)
        assert isinstance(binary, stipy.BinaryData)
        assert binary.bytes() == len(payload)
        assert binary.wordsize() == 1
        assert binary.hasStream()
        assert not binary.hasLocalData()

        assert binary.pull()
        assert binary.getBytes() == payload


def test_stiserver_proxy_readwrite_channel_17_binary_read_does_not_crash_server(
    sti_nameservice_address,
    stipy_modules,
    tmp_path,
):
    stipy, stidevicepy = stipy_modules

    payload = (
        b"readWrite plain BinaryData payload\n"
        b"This channel returns BinaryData that is not an Image.\n"
    )
    server_name = "stidevicepy STIServer Binary Server"
    server_address = "localhost"
    server_module = 59
    server_id = stipy.DeviceID(server_name, server_address, server_module)
    config_path = _server_config(tmp_path, sti_nameservice_address, server_name, server_address, server_module)

    stiserver = StiServerProcess(_stiserver_binary(), config_path, sti_nameservice_address).start()
    device_hub = None
    try:
        server_ref = {"device": None}

        def connect_server():
            stiserver.assert_running()
            server_ref["device"] = stipy.connect(server_id, sti_nameservice_address)
            return server_ref["device"] is not None

        wait_for(
            connect_server,
            timeout_s=10.0,
            describe=lambda: "STIServer was not reachable",
            diagnostics=stiserver.diagnostics,
        )

        device_spec = DeviceSpec(
            name="TestDevice",
            address="localhost",
            module=0,
            target_server_id=server_id.getID(),
            output_channels=[],
            input_channels=[
                ChannelSpec(17, name="example binary data", value_type=stipy.MixedValueType.Binary, direction="input"),
            ],
        )
        device = ReadWriteBinaryChannelDevice(stidevicepy, device_spec, payload)
        device_hub = stidevicepy.NetworkDeviceHub(sti_nameservice_address)
        device_hub.addDevice(device)
        device_hub.run(False)

        wait_for_device_ids(
            device_hub,
            [device_spec.device_id()],
            timeout_s=10.0,
            diagnostics=stiserver.diagnostics,
        )

        server = server_ref["device"]
        wait_for(
            lambda: server.getDeviceCollection().contains(device_spec.device_id()),
            timeout_s=10.0,
            describe=lambda: "STIServer did not collect TestDevice",
            diagnostics=stiserver.diagnostics,
        )

        remote_device = server.getDeviceCollection().get(device_spec.device_id())
        binary = remote_device.read(17)
        stiserver.assert_running()

        assert isinstance(binary, stipy.BinaryData)
        assert binary.bytes() == len(payload)
        assert binary.wordsize() == 1
        assert binary.hasStream()
        assert not binary.hasLocalData()

        assert binary.pull()
        stiserver.assert_running()
        assert binary.getBytes() == payload
    finally:
        if device_hub is not None:
            try:
                device_hub.shutdown()
            except Exception:
                pass
            try:
                device_hub.disconnect()
            except Exception:
                pass
        stiserver.shutdown()


def test_stiserver_proxy_collects_pre_registration_binary_last_measurement(
    sti_nameservice_address,
    stipy_modules,
    tmp_path,
):
    stipy, stidevicepy = stipy_modules

    payload = (
        b"readWrite plain BinaryData payload\n"
        b"This channel was read before the device joined the network.\n"
    )
    server_name = "stidevicepy STIServer Cached Binary Server"
    server_address = "localhost"
    server_module = 60
    server_id = stipy.DeviceID(server_name, server_address, server_module)
    config_path = _server_config(tmp_path, sti_nameservice_address, server_name, server_address, server_module)

    stiserver = StiServerProcess(_stiserver_binary(), config_path, sti_nameservice_address).start()
    device_hub = None
    try:
        server_ref = {"device": None}

        def connect_server():
            stiserver.assert_running()
            server_ref["device"] = stipy.connect(server_id, sti_nameservice_address)
            return server_ref["device"] is not None

        wait_for(
            connect_server,
            timeout_s=10.0,
            describe=lambda: "STIServer was not reachable",
            diagnostics=stiserver.diagnostics,
        )

        device_spec = DeviceSpec(
            name="TestDevice With Cached Binary",
            address="localhost",
            module=61,
            target_server_id=server_id.getID(),
            output_channels=[],
            input_channels=[
                ChannelSpec(17, name="example binary data", value_type=stipy.MixedValueType.Binary, direction="input"),
            ],
        )
        device = ReadWriteBinaryChannelDevice(stidevicepy, device_spec, payload)

        local_binary = device.read(17)
        assert isinstance(local_binary, stipy.BinaryData)
        assert local_binary.getBytes() == payload

        device_hub = stidevicepy.NetworkDeviceHub(sti_nameservice_address)
        device_hub.addDevice(device)
        device_hub.run(False)

        wait_for_device_ids(
            device_hub,
            [device_spec.device_id()],
            timeout_s=10.0,
            diagnostics=stiserver.diagnostics,
        )

        server = server_ref["device"]
        wait_for(
            lambda: server.getDeviceCollection().contains(device_spec.device_id()),
            timeout_s=10.0,
            describe=lambda: "STIServer did not collect TestDevice With Cached Binary",
            diagnostics=stiserver.diagnostics,
        )
        stiserver.assert_running()

        local_binary = device.read(17)
        assert isinstance(local_binary, stipy.BinaryData)
        assert local_binary.getBytes() == payload
        time.sleep(1.0)
        stiserver.assert_running()

        remote_device = server.getDeviceCollection().get(device_spec.device_id())
        channel = remote_device.getChannelManager().getChannel(17)
        measurement = channel.getLastMeasurement()
        stiserver.assert_running()

        assert measurement.getType() == stipy.MixedValueType.Binary
        binary = measurement.getBinary()
        assert binary is not None
        assert binary.bytes() == len(payload)
        assert binary.wordsize() == 1
        assert binary.hasStream()
        assert not binary.hasLocalData()

        assert binary.pull()
        stiserver.assert_running()
        assert binary.getBytes() == payload
    finally:
        if device_hub is not None:
            try:
                device_hub.shutdown()
            except Exception:
                pass
            try:
                device_hub.disconnect()
            except Exception:
                pass
        stiserver.shutdown()


def test_stiserver_proxy_collects_file_transfer_pre_registration_binary_measurement(
    sti_nameservice_address,
    stipy_modules,
    tmp_path,
):
    stipy, stidevicepy = stipy_modules

    server_name = "stidevicepy STIServer FileTransfer Binary Server"
    server_address = "localhost"
    server_module = 62
    server_id = stipy.DeviceID(server_name, server_address, server_module)
    config_path = _server_config(tmp_path, sti_nameservice_address, server_name, server_address, server_module)

    stiserver = StiServerProcess(_stiserver_binary(), config_path, sti_nameservice_address).start()
    device_hub = None
    try:
        server_ref = {"device": None}

        def connect_server():
            stiserver.assert_running()
            server_ref["device"] = stipy.connect(server_id, sti_nameservice_address)
            return server_ref["device"] is not None

        wait_for(
            connect_server,
            timeout_s=10.0,
            describe=lambda: "STIServer was not reachable",
            diagnostics=stiserver.diagnostics,
        )

        device_spec = DeviceSpec(
            name="FileTransferDevice",
            address="localhost",
            module=63,
            target_server_id=server_id.getID(),
            output_channels=[
                ChannelSpec(6, name="uploaded file (FileID)", value_type=stipy.MixedValueType.File, direction="output"),
            ],
            input_channels=[
                ChannelSpec(13, name="random image (BinaryData)", value_type=stipy.MixedValueType.Image, direction="input"),
                ChannelSpec(14, name="random raw image (FileHolder)", value_type=stipy.MixedValueType.Image, direction="input"),
                ChannelSpec(15, name="random multi-pane TIF image (FileHolder)", value_type=stipy.MixedValueType.Image, direction="input"),
                ChannelSpec(16, name="example text file (FileHolder)", value_type=stipy.MixedValueType.File, direction="input"),
                ChannelSpec(17, name="example binary data", value_type=stipy.MixedValueType.Binary, direction="input"),
                ChannelSpec(18, name="example virtual text file (VirtualFileHolder)", value_type=stipy.MixedValueType.File, direction="input"),
                ChannelSpec(19, name="inverted image", value_type=stipy.MixedValueType.Image, direction="input"),
                ChannelSpec(20, name="uploaded file size", value_type=stipy.MixedValueType.Int, direction="input"),
                ChannelSpec(21, name="image vector measurement", value_type=stipy.MixedValueType.Vector, direction="input"),
            ],
        )
        device = FileTransferSequenceDevice(stidevicepy, device_spec)

        assert isinstance(device.read(13), stipy.Image)
        assert isinstance(device.read(14), stipy.Image)
        assert isinstance(device.read(15), stipy.Image)
        assert isinstance(device.read(16), stipy.FileID)
        local_binary = device.read(17)
        assert isinstance(local_binary, stipy.BinaryData)
        assert local_binary.getBytes().startswith(b"fileTransfer plain BinaryData payload")
        assert isinstance(device.read(18), stipy.FileID)

        input_image = stipy.Image(device.random_image_bytes(), device.IMAGE_WIDTH, device.IMAGE_HEIGHT)
        assert isinstance(device.read(19, input_image), stipy.Image)
        assert device.read(21) is not None

        persistence = device.getPersistenceManager()
        if hasattr(persistence, "importFile"):
            source_holder, source_server, import_payload = make_import_source_for_test(persistence, tmp_path)
            assert source_holder is not None
            assert source_server is not None
            with persistence.importFile(source_holder.getID(), source_server) as imported:
                assert device.write(6, imported.fileID)
                assert device.read(20, imported.fileID) == len(import_payload)

        device_hub = stidevicepy.NetworkDeviceHub(sti_nameservice_address)
        device_hub.addDevice(device)
        device_hub.run(False)

        wait_for_device_ids(
            device_hub,
            [device_spec.device_id()],
            timeout_s=10.0,
            diagnostics=stiserver.diagnostics,
        )

        server = server_ref["device"]
        wait_for(
            lambda: server.getDeviceCollection().contains(device_spec.device_id()),
            timeout_s=10.0,
            describe=lambda: "STIServer did not collect FileTransferDevice",
            diagnostics=stiserver.diagnostics,
        )
        stiserver.assert_running()

        local_binary = device.read(17)
        assert isinstance(local_binary, stipy.BinaryData)
        assert local_binary.getBytes().startswith(b"fileTransfer plain BinaryData payload")
        time.sleep(1.0)
        stiserver.assert_running()

        remote_device = server.getDeviceCollection().get(device_spec.device_id())
        channel = remote_device.getChannelManager().getChannel(17)
        measurement = channel.getLastMeasurement()
        stiserver.assert_running()

        assert measurement.getType() == stipy.MixedValueType.Binary
        binary = measurement.getBinary()
        assert binary is not None
        assert binary.bytes() == local_binary.bytes()
        assert binary.hasStream()
        assert not binary.hasLocalData()

        assert binary.pull()
        stiserver.assert_running()
        assert binary.getBytes() == local_binary.getBytes()
    finally:
        if device_hub is not None:
            try:
                device_hub.shutdown()
            except Exception:
                pass
            try:
                device_hub.disconnect()
            except Exception:
                pass
        stiserver.shutdown()


def test_stidevicepy_can_save_file_backed_image_last_measurements(
    sti_nameservice_address,
    stipy_modules,
    tmp_path,
):
    stipy, stidevicepy = stipy_modules

    server_spec = make_server_spec(name="stidevicepy File Image Server", address="localhost", module=54)
    server_id = server_spec.device_id()
    device_spec = DeviceSpec(
        name="stidevicepy File Image Device",
        address="localhost",
        module=55,
        target_server_id=server_id.getID(),
        output_channels=[],
        input_channels=[
            ChannelSpec(14, name="raw file image", value_type=stipy.MixedValueType.Image, direction="input"),
            ChannelSpec(15, name="tif file image", value_type=stipy.MixedValueType.Image, direction="input"),
        ],
    )

    with InProcessTopology(sti_nameservice_address, server_spec, []) as topology:
        device = FileBackedImageReadDevice(stidevicepy, device_spec)
        topology.devices.append(device)
        topology.hub.addDevice(device)

        wait_for_device_ids(
            topology.hub,
            [server_id, device_spec.device_id()],
            timeout_s=5.0,
            diagnostics=topology.diagnostics,
        )

        server = topology.connect_stipy()
        assert server is not None, topology.summary()
        remote_device = server.getDeviceCollection().get(device_spec.device_id())

        assert isinstance(remote_device.read(14), stipy.Image)
        raw_image = last_image(remote_device, 14, stipy)
        assert not raw_image.hasData()
        assert not raw_image.hasFile()
        assert raw_image.getFileID().filename.endswith(".raw")

        raw_output_path = tmp_path / "readWrite-image.raw"
        save_image_file_to_path(stipy, remote_device, raw_image, raw_output_path)
        assert raw_output_path.read_bytes() == bytes((index % 256 for index in range(100)))

        assert isinstance(remote_device.read(15), stipy.Image)
        tif_image = last_image(remote_device, 15, stipy)
        assert not tif_image.hasData()
        assert not tif_image.hasFile()
        assert tif_image.getFileID().filename.endswith(".tif")

        tif_output_path = tmp_path / "readWrite-image.tif"
        save_image_file_to_path(stipy, remote_device, tif_image, tif_output_path)
        assert tif_output_path.read_bytes().startswith((b"II*\x00", b"MM\x00*"))


def test_stidevicepy_can_receive_tif_image_into_virtual_file_server(
    sti_nameservice_address,
    stipy_modules,
):
    stipy, stidevicepy = stipy_modules

    server_spec = make_server_spec(name="stidevicepy Virtual File Image Server", address="localhost", module=56)
    server_id = server_spec.device_id()
    device_spec = DeviceSpec(
        name="stidevicepy Virtual File Image Device",
        address="localhost",
        module=57,
        target_server_id=server_id.getID(),
        output_channels=[],
        input_channels=[
            ChannelSpec(15, name="tif file image", value_type=stipy.MixedValueType.Image, direction="input"),
        ],
    )

    with InProcessTopology(sti_nameservice_address, server_spec, []) as topology:
        device = FileBackedImageReadDevice(stidevicepy, device_spec)
        topology.devices.append(device)
        topology.hub.addDevice(device)

        wait_for_device_ids(
            topology.hub,
            [server_id, device_spec.device_id()],
            timeout_s=5.0,
            diagnostics=topology.diagnostics,
        )

        server = topology.connect_stipy()
        assert server is not None, topology.summary()
        remote_device = server.getDeviceCollection().get(device_spec.device_id())

        assert isinstance(remote_device.read(15), stipy.Image)
        tif_image = last_image(remote_device, 15, stipy)
        assert not tif_image.hasData()
        assert not tif_image.hasFile()
        assert tif_image.getFileID().filename.endswith(".tif")

        received_bytes, roundtrip_bytes = receive_image_file_to_virtual_server(stipy, remote_device, tif_image)
        assert received_bytes.startswith((b"II*\x00", b"MM\x00*"))
        assert roundtrip_bytes == received_bytes
