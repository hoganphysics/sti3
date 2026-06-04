"""stidevicepy runtime coverage for lazy binary channel measurements."""

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
