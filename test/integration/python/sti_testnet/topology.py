"""Topology builders and lifecycle handles for simulated STI networks."""

from .devices import SimulatedDevice
from .devices import make_server_spec
from .devices import require_stipy


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
    def __init__(self, nameservice_address, server_spec=None, device_specs=None, hub_id=None):
        self.nameservice_address = nameservice_address
        self.server_spec = server_spec or make_server_spec()
        self.device_specs = list(device_specs or [])
        self.hub_id = hub_id
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
        if self.hub_id is None:
            self.hub = stidevicepy.NetworkDeviceHub(self.nameservice_address)
        else:
            self.hub = stidevicepy.NetworkDeviceHub(self.hub_id, self.nameservice_address)

        self.server = SimulatedDevice(self.server_spec)
        self.hub.addDevice(self.server)
        self.devices = []
        for spec in self.device_specs:
            device = SimulatedDevice(spec)
            self.devices.append(device)
            self.hub.addDevice(device)

        self.hub.run(False)
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
        lines.append("Known devices:")
        if self.server is not None:
            lines.append("  {0}".format(self.server.getID().getID()))
        for device in self.devices:
            lines.append("  {0}".format(device.getID().getID()))
        return "\n".join(lines)


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
