"""Simulated Python devices for STI network integration tests."""

import json
import os
import threading
import time


try:
    import stipy
    import stipy.stidevicepy as stidevicepy
except ImportError as exc:
    stipy = None
    stidevicepy = None
    STIPY_IMPORT_ERROR = exc
else:
    STIPY_IMPORT_ERROR = None


def require_stipy():
    if stipy is None or stidevicepy is None:
        raise RuntimeError("stipy is required for integration test devices: {0}".format(STIPY_IMPORT_ERROR))
    return stipy, stidevicepy


class ChannelSpec(object):
    def __init__(self, number, name=None, value_type="Number", direction="output"):
        self.number = int(number)
        self.name = name or "{0} channel {1}".format(direction, number)
        self.value_type = value_type
        self.direction = direction

    def to_dict(self):
        return {
            "number": self.number,
            "name": self.name,
            "value_type": self.value_type,
            "direction": self.direction,
        }


class DeviceBehavior(object):
    def __init__(
        self,
        parse_delay_s=0.0,
        load_delay_s=0.0,
        play_delay_s=0.0,
        collect_delay_s=0.0,
        parse_error=None,
        load_error=None,
        play_error=None,
        collect_error=None,
        measurement_value=1.0,
    ):
        self.parse_delay_s = float(parse_delay_s)
        self.load_delay_s = float(load_delay_s)
        self.play_delay_s = float(play_delay_s)
        self.collect_delay_s = float(collect_delay_s)
        self.parse_error = parse_error
        self.load_error = load_error
        self.play_error = play_error
        self.collect_error = collect_error
        self.measurement_value = measurement_value

    def to_dict(self):
        return {
            "parse_delay_s": self.parse_delay_s,
            "load_delay_s": self.load_delay_s,
            "play_delay_s": self.play_delay_s,
            "collect_delay_s": self.collect_delay_s,
            "parse_error": self.parse_error,
            "load_error": self.load_error,
            "play_error": self.play_error,
            "collect_error": self.collect_error,
            "measurement_value": self.measurement_value,
        }


class DeviceSpec(object):
    def __init__(
        self,
        name,
        address="localhost",
        module=0,
        target_server_id="",
        output_channels=None,
        input_channels=None,
        behavior=None,
        event_targets=None,
        partners=None,
        persistence_root=None,
        device_subdirectory=None,
        record_path=None,
    ):
        self.name = name
        self.address = address
        self.module = int(module)
        self.target_server_id = target_server_id
        if output_channels is None:
            output_channels = [ChannelSpec(0, "output 0", "Number", "output")]
        if input_channels is None:
            input_channels = []
        self.output_channels = list(output_channels)
        self.input_channels = list(input_channels)
        self.behavior = behavior or DeviceBehavior()
        self.event_targets = list(event_targets or [])
        self.partners = list(partners or [])
        self.persistence_root = persistence_root
        self.device_subdirectory = device_subdirectory
        self.record_path = record_path

    def device_id(self):
        require_stipy()
        return stipy.DeviceID(self.name, self.address, self.module, self.target_server_id)

    def config(self):
        require_stipy()
        config = stipy.Configuration()
        config.set("Device Name", self.name)
        config.set("IP Address", self.address)
        config.set("Module", str(self.module))
        config.set("Target Server", self.target_server_id)
        if self.persistence_root is not None:
            config.set("PersistenceManager", "root path", self.persistence_root)
        if self.device_subdirectory is not None:
            config.set("PersistenceManager", "device subdirectory", self.device_subdirectory)
        return config

    def to_dict(self):
        return {
            "name": self.name,
            "address": self.address,
            "module": self.module,
            "target_server_id": self.target_server_id,
            "output_channels": [channel.to_dict() for channel in self.output_channels],
            "input_channels": [channel.to_dict() for channel in self.input_channels],
            "behavior": self.behavior.to_dict(),
            "event_targets": [_device_id_text(target) for target in self.event_targets],
            "partners": [_device_id_text(partner) for partner in self.partners],
            "persistence_root": self.persistence_root,
            "device_subdirectory": self.device_subdirectory,
            "record_path": self.record_path,
        }


class EventRecord(object):
    def __init__(self, phase, time_ns, channel=None, value=None):
        self.phase = phase
        self.time_ns = time_ns
        self.channel = channel
        self.value = value

    def __repr__(self):
        return "EventRecord({0}, time={1}, channel={2}, value={3})".format(
            self.phase,
            self.time_ns,
            self.channel,
            self.value,
        )

    def to_dict(self):
        return {
            "phase": self.phase,
            "time_ns": self.time_ns,
            "channel": self.channel,
            "value": _jsonable_value(self.value),
        }


def channel_spec_from_dict(data):
    return ChannelSpec(
        data["number"],
        name=data.get("name"),
        value_type=data.get("value_type", "Number"),
        direction=data.get("direction", "output"),
    )


def behavior_from_dict(data):
    data = data or {}
    return DeviceBehavior(
        parse_delay_s=data.get("parse_delay_s", 0.0),
        load_delay_s=data.get("load_delay_s", 0.0),
        play_delay_s=data.get("play_delay_s", 0.0),
        collect_delay_s=data.get("collect_delay_s", 0.0),
        parse_error=data.get("parse_error"),
        load_error=data.get("load_error"),
        play_error=data.get("play_error"),
        collect_error=data.get("collect_error"),
        measurement_value=data.get("measurement_value", 1.0),
    )


def device_spec_from_dict(data):
    return DeviceSpec(
        name=data["name"],
        address=data.get("address", "localhost"),
        module=data.get("module", 0),
        target_server_id=data.get("target_server_id", ""),
        output_channels=[channel_spec_from_dict(item) for item in data.get("output_channels", [])],
        input_channels=[channel_spec_from_dict(item) for item in data.get("input_channels", [])],
        behavior=behavior_from_dict(data.get("behavior")),
        event_targets=[_device_id_from_text(item) for item in data.get("event_targets", [])],
        partners=[_device_id_from_text(item) for item in data.get("partners", [])],
        persistence_root=data.get("persistence_root"),
        device_subdirectory=data.get("device_subdirectory"),
        record_path=data.get("record_path"),
    )


def event_record_from_dict(data):
    return EventRecord(
        data.get("phase"),
        data.get("time_ns"),
        channel=data.get("channel"),
        value=data.get("value"),
    )


def _device_id_text(device_id):
    if hasattr(device_id, "getID"):
        return device_id.getID()
    return str(device_id)


def _device_id_from_text(device_id_text):
    require_stipy()
    return stipy.DeviceID(device_id_text)


def _jsonable_value(value):
    try:
        json.dumps(value)
        return value
    except TypeError:
        return repr(value)


def make_server_spec(name="STI Server", address="localhost", module=0):
    return DeviceSpec(
        name=name,
        address=address,
        module=module,
        target_server_id="",
        output_channels=[],
        input_channels=[],
    )


def make_device_spec(name, server_id, address="localhost", module=1, output_channels=None, input_channels=None, behavior=None):
    if hasattr(server_id, "getID"):
        target_server_id = server_id.getID()
    else:
        target_server_id = str(server_id)
    return DeviceSpec(
        name=name,
        address=address,
        module=module,
        target_server_id=target_server_id,
        output_channels=output_channels,
        input_channels=input_channels,
        behavior=behavior,
    )


def mixed_value_type(name_or_type):
    require_stipy()
    if not isinstance(name_or_type, str):
        return name_or_type
    try:
        return getattr(stipy.MixedValueType, name_or_type)
    except AttributeError:
        raise ValueError("Unknown MixedValueType: {0}".format(name_or_type))


if stidevicepy is not None:
    class SimulatedOutputEvent(stidevicepy.SynchronousEvent):
        def __init__(self, event_time, device, raw_events):
            stidevicepy.SynchronousEvent.__init__(self, event_time)
            self.device = device
            self.values = []
            for event in raw_events:
                self.values.append((event.channel(), event.value()))

        def loadEvent(self):
            behavior = self.device.spec.behavior
            if behavior.load_delay_s > 0:
                time.sleep(behavior.load_delay_s)
            for channel, value in self.values:
                self.device.record("load", self.getTime(), channel, value)
            if behavior.load_error:
                self.addError("Simulated load error").appendMessage(str(behavior.load_error))

        def playEvent(self):
            behavior = self.device.spec.behavior
            if behavior.play_delay_s > 0:
                time.sleep(behavior.play_delay_s)
            for channel, value in self.values:
                self.device.record("play", self.getTime(), channel, value)
            if behavior.play_error:
                self.addError("Simulated play error").appendMessage(str(behavior.play_error))

        def collectMeasurementData(self):
            return

        def stopEvent(self):
            self.device.record("stop", self.getTime())

        def pauseEvent(self):
            self.device.record("pause", self.getTime())

        def unpauseEvent(self, retrigger):
            self.device.record("unpause", self.getTime(), value=retrigger)


    class SimulatedMeasurementEvent(stidevicepy.SynchronousEvent):
        def __init__(self, event_time, device, raw_events):
            stidevicepy.SynchronousEvent.__init__(self, event_time)
            self.device = device
            for event in raw_events:
                self.addMeasurement(event)

        def loadEvent(self):
            self.device.record("load-measurement", self.getTime())

        def playEvent(self):
            self.device.record("play-measurement", self.getTime())

        def collectMeasurementData(self):
            behavior = self.device.spec.behavior
            if behavior.collect_delay_s > 0:
                time.sleep(behavior.collect_delay_s)
            if behavior.collect_error:
                self.addError("Simulated collect error").appendMessage(str(behavior.collect_error))
                return
            for measurement in self.getMeasurements():
                self.device.record("collect", self.getTime(), measurement.channel(), behavior.measurement_value)
                measurement.setMeasurementResult(behavior.measurement_value)

        def stopEvent(self):
            self.device.record("stop-measurement", self.getTime())

        def pauseEvent(self):
            self.device.record("pause-measurement", self.getTime())

        def unpauseEvent(self, retrigger):
            self.device.record("unpause-measurement", self.getTime(), value=retrigger)


    class SimulatedDevice(stidevicepy.LocalDevice):
        def __init__(self, spec):
            self.spec = spec
            self.records = []
            self._records_lock = threading.Lock()
            stidevicepy.LocalDevice.__init__(self, spec.config())

            for channel in spec.output_channels:
                self.addOutputChannel(channel.number, mixed_value_type(channel.value_type), channel.name)
            for channel in spec.input_channels:
                self.addInputChannel(channel.number, mixed_value_type(channel.value_type), channel.name)
            for target in spec.event_targets:
                self.addEventTarget(target)
            for partner in spec.partners:
                self.addPartner(partner)

        def record(self, phase, event_time, channel=None, value=None):
            record = EventRecord(phase, event_time, channel, value)
            with self._records_lock:
                self.records.append(record)
                if self.spec.record_path is not None:
                    self._append_record(record)

        def records_for(self, phase):
            with self._records_lock:
                return [record for record in self.records if record.phase == phase]

        def clear_records(self):
            with self._records_lock:
                self.records = []

        def _append_record(self, record):
            directory = os.path.dirname(self.spec.record_path)
            if directory:
                os.makedirs(directory, exist_ok=True)
            with open(self.spec.record_path, "a") as handle:
                handle.write(json.dumps(record.to_dict(), sort_keys=True))
                handle.write("\n")

        def parseEvents(self, eventsIn, synchedEvents):
            behavior = self.spec.behavior
            if behavior.parse_delay_s > 0:
                time.sleep(behavior.parse_delay_s)

            event_groups = list(eventsIn.items())
            if behavior.parse_error:
                first_event = None
                for _, events in event_groups:
                    if events:
                        first_event = events[0]
                        break
                if first_event is not None:
                    raise stidevicepy.EventParsingException(first_event, str(behavior.parse_error))
                raise RuntimeError(str(behavior.parse_error))

            for event_time, events in event_groups:
                measurement_events = []
                output_events = []
                for event in events:
                    if event.isMeasurementEvent():
                        measurement_events.append(event)
                    else:
                        output_events.append(event)

                if output_events:
                    synchedEvents.append(SimulatedOutputEvent(event_time, self, output_events))
                if measurement_events:
                    synchedEvents.append(SimulatedMeasurementEvent(event_time, self, measurement_events))
else:
    class SimulatedDevice(object):
        def __init__(self, spec):
            require_stipy()
