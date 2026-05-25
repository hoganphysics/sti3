"""Child-process entry point for simulated integration-test devices."""

import argparse
import json
import signal
import sys
import threading
import time
import traceback

from .devices import SimulatedDevice
from .devices import device_spec_from_dict
from .devices import require_stipy


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--nameservice", required=True)
    parser.add_argument("--spec", required=True)
    args = parser.parse_args(argv)

    stop_requested = threading.Event()

    def request_stop(signum, frame):
        stop_requested.set()

    signal.signal(signal.SIGTERM, request_stop)
    signal.signal(signal.SIGINT, request_stop)

    hub = None
    try:
        _, stidevicepy = require_stipy()
        with open(args.spec, "r") as handle:
            spec = device_spec_from_dict(json.load(handle))

        hub = stidevicepy.NetworkDeviceHub(args.nameservice)
        device = SimulatedDevice(spec)
        hub.addDevice(device)
        hub.run(False)

        sys.stdout.write("STI_TESTNET_DEVICE_READY {0}\n".format(device.getID().getID()))
        sys.stdout.flush()

        while not stop_requested.is_set():
            time.sleep(0.1)
    except Exception:
        traceback.print_exc()
        sys.stdout.flush()
        sys.stderr.flush()
        return 1
    finally:
        if hub is not None:
            try:
                hub.shutdown()
            except Exception:
                pass
            try:
                hub.disconnect()
            except Exception:
                pass

    return 0


if __name__ == "__main__":
    sys.exit(main())
