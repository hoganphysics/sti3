"""Run a small device network for manual UI integration testing."""

import argparse
import os
import threading

import stipy
import stipy.stidevicepy as stidevicepy


class EventChecker(stidevicepy.LocalDevice):
    def __init__(self, config, scheduler_server):
        stidevicepy.LocalDevice.__init__(self, config)
        self.events = []
        self._lock = threading.Lock()
        self.addOutputChannel(0, stipy.MixedValueType.Number, "Start Event Checker")
        self.addOutputChannel(1, stipy.MixedValueType.Number, "End Event Checker")

        server_id = stipy.DeviceID(scheduler_server)
        self.addPartner(server_id, "server")
        self.getMessageReceiver().addListener(
            stipy.DeviceMessageType.EngineScheduler,
            server_id,
            "EngineSchedulerUpdates",
            self.schedulerUpdates,
        )

    def schedulerUpdates(self, message):
        print("Scheduler update:")
        print(message)

    def writeChannel(self, channel, value):
        with self._lock:
            if channel == 0:
                self.events = []
            elif channel == 1:
                print(self.events)

        return True

    def onEvent(self, event):
        with self._lock:
            self.events.append(event)


class EventCheckingDevice(stidevicepy.LocalDevice):
    def __init__(self, config, checker: EventChecker):
        stidevicepy.LocalDevice.__init__(self, config)
        self.checker = checker
        self.addAttribute("Enable Trigger", "On", ["On", "Off"])

    def writeChannel(self, channel, value):
        event = f"Write to {self.getID().getID()} channel {channel}: {value}"
        print(event)
        self.checker.onEvent(event)
        return True

    def readChannel(self, channel, value):
        return None


def device_config(name, target_server):
    return stipy.Configuration(
        {
            "Device Name": name,
            "IP Address": "localhost",
            "Module": "0",
            "Target Server": target_server,
        }
    )


def add_output_channels(device, count):
    for channel in range(count):
        device.addOutputChannel(
            channel,
            stipy.MixedValueType.Double,
            f"Output {channel}",
        )


def build_hub(name_service, target_server, scheduler_server):
    hub = stidevicepy.NetworkDeviceHub(name_service)

    checker = EventChecker(
        device_config("EventCheckingDevice", target_server),
        scheduler_server,
    )
    hub.addDevice(checker)

    for name, channel_count in (("Device1", 2), ("Device2", 1), ("Device3", 3)):
        device = EventCheckingDevice(device_config(name, target_server), checker)
        add_output_channels(device, channel_count)
        device.addVersionInfo("UI Test Device", "1.0.0")
        hub.addDevice(device)

    return hub


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--name-service",
        default=os.environ.get("STI_NAME_SERVICE", "localhost:2809"),
        help="omniORB name service address (default: %(default)s)",
    )
    parser.add_argument(
        "--target-server",
        default=os.environ.get("STI_TARGET_SERVER", "localhost/0/STI Server"),
        help="DeviceID of the target STI server (default: %(default)s)",
    )
    parser.add_argument(
        "--scheduler-server",
        default=os.environ.get("STI_SCHEDULER_SERVER"),
        help="DeviceID that emits scheduler updates (default: target server)",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    scheduler_server = args.scheduler_server or args.target_server
    hub = build_hub(args.name_service, args.target_server, scheduler_server)
    hub.run(True)


if __name__ == "__main__":
    main()
