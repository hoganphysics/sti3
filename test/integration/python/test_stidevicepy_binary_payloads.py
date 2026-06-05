"""stidevicepy runtime coverage for lazy binary channel measurements."""

import struct

import pytest

from sti_testnet.devices import ChannelSpec
from sti_testnet.devices import DeviceSpec
from sti_testnet.devices import make_server_spec
from sti_testnet.topology import InProcessTopology
from sti_testnet.waits import wait_for_device_ids
from sti_testnet.waits import wait_for_ticket


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice]


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
                file_holder = persistence.makeFileHolder("", filename)

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

    virtual_file_server.addFile(backing_destination)
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
