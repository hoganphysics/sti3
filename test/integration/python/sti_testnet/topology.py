"""Topology builders and lifecycle handles for simulated STI networks."""

import gc
import shutil
import tempfile

from .devices import SimulatedDevice
from .devices import make_server_spec
from .devices import require_stipy


_OWNED_PERSISTENCE_ROOTS = set()


def cleanup_registered_persistence_roots():
    for root in list(_OWNED_PERSISTENCE_ROOTS):
        shutil.rmtree(root, ignore_errors=True)
        if not _path_exists(root):
            _OWNED_PERSISTENCE_ROOTS.discard(root)


def _path_exists(path):
    try:
        import os
        return os.path.exists(path)
    except Exception:
        return True


class FrontendConnectionInfo(object):
    def __init__(self, nameservice_address, server_id):
        self.nameservice_address = nameservice_address
        self.server_id = server_id

    @property
    def nameservice_host(self):
        if ":" in self.nameservice_address:
            return self.nameservice_address.rsplit(":", 1)[0]
        return self.nameservice_address

    @property
    def nameservice_port(self):
        if ":" in self.nameservice_address:
            return self.nameservice_address.rsplit(":", 1)[1]
        return ""

    def lines(self):
        server_id = self.server_id
        return [
            "Name service: {0}".format(self.nameservice_address),
            "Name service host: {0}".format(self.nameservice_host),
            "Name service port: {0}".format(self.nameservice_port),
            "Server DeviceID: {0}".format(server_id.getID()),
            "Server name: {0}".format(server_id.name()),
            "Server address: {0}".format(server_id.address()),
            "Server module: {0}".format(server_id.module()),
        ]

    def __str__(self):
        return "\n".join(self.lines())


class InProcessTopology(object):
    def __init__(self, nameservice_address, server_spec=None, device_specs=None, hub_id=None, persistence_root=None):
        self.nameservice_address = nameservice_address
        self.server_spec = server_spec or make_server_spec()
        self.device_specs = list(device_specs or [])
        self.hub_id = hub_id
        self.persistence_root = persistence_root
        self._owns_persistence_root = persistence_root is None
        self.hub = None
        self.server = None
        self.devices = []
        self.started = False

    @property
    def server_id(self):
        if self.server is not None:
            return self.server.getID()
        return self.server_spec.device_id()

    @property
    def frontend(self):
        return FrontendConnectionInfo(self.nameservice_address, self.server_id)

    def start(self):
        _, stidevicepy = require_stipy()
        if self.persistence_root is None:
            self.persistence_root = tempfile.mkdtemp(prefix="sti3-integration-")
            _OWNED_PERSISTENCE_ROOTS.add(self.persistence_root)
        self._apply_persistence_root()

        if self.hub_id is None:
            self.hub = stidevicepy.NetworkDeviceHub(self.nameservice_address)
        else:
            self.hub = stidevicepy.NetworkDeviceHub(self.hub_id, self.nameservice_address)

        self.server = SimulatedDevice(self.server_spec)
        self.hub.addDevice(self.server)
        self.hub.run(False)

        self.devices = []
        for spec in self.device_specs:
            device = SimulatedDevice(spec)
            self.devices.append(device)
            self.hub.addDevice(device)

        self.started = True
        return self

    def shutdown(self):
        if self.hub is not None:
            try:
                self.hub.shutdown()
            finally:
                try:
                    self.hub.disconnect()
                except Exception:
                    pass
        self.hub = None
        self.server = None
        self.devices = []
        gc.collect()
        if self._owns_persistence_root and self.persistence_root is not None:
            shutil.rmtree(self.persistence_root, ignore_errors=True)
            self.persistence_root = None
        self.started = False

    def __enter__(self):
        return self.start()

    def __exit__(self, exc_type, exc, tb):
        self.shutdown()

    def connect_stipy(self):
        stipy, _ = require_stipy()
        return stipy.connect(self.server_id, self.nameservice_address)

    def print_network(self):
        if self.hub is None:
            return "<topology not started>"
        return self.hub.printNetwork()

    def summary(self):
        lines = self.frontend.lines()
        lines.append("Persistence root: {0}".format(self.persistence_root))
        lines.append("Known devices:")
        if self.server is not None:
            lines.append("  {0}".format(self.server.getID().getID()))
        for device in self.devices:
            lines.append("  {0}".format(device.getID().getID()))
        return "\n".join(lines)

    def diagnostics(self):
        lines = [self.summary()]
        if self.hub is not None:
            try:
                lines.append("Hub DeviceIDs: {0}".format([device_id.getID() for device_id in self.hub.getDeviceIDs()]))
            except Exception as exc:
                lines.append("Hub DeviceIDs unavailable: {0}".format(exc))
            try:
                lines.append("Network:\n{0}".format(self.hub.printNetwork()))
            except Exception as exc:
                lines.append("Network summary unavailable: {0}".format(exc))
        for device in self.devices:
            lines.append("Records for {0}: {1}".format(device.getID().getID(), device.records))
        return "\n".join(lines)

    def _apply_persistence_root(self):
        specs = [self.server_spec] + self.device_specs
        for spec in specs:
            if spec.persistence_root is None:
                spec.persistence_root = self.persistence_root


class ProcessTopology(object):
    def __init__(self, *args, **kwargs):
        self.args = args
        self.kwargs = kwargs

    def start(self):
        raise NotImplementedError("ProcessTopology is reserved for the next harness pass.")

    def shutdown(self):
        return

    def __enter__(self):
        return self.start()

    def __exit__(self, exc_type, exc, tb):
        self.shutdown()
