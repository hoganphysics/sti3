"""Partner-device reference distribution integration scenarios."""

import shutil
import tempfile
import time

import pytest

from sti_testnet.devices import DeviceSpec
from sti_testnet.devices import SimulatedDevice
from sti_testnet.devices import make_server_spec
from sti_testnet.shots import single_device_output
from sti_testnet.topology import InProcessTopology
from sti_testnet.topology import ProcessTopology
from sti_testnet.waits import wait_for
from sti_testnet.waits import wait_for_ticket


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice]


def _device_id_without_target(stipy, spec):
    return stipy.DeviceID(spec.name, spec.address, spec.module)


def _make_child_spec(name, module, server_id, partners=None):
    return DeviceSpec(
        name=name,
        address="localhost",
        module=module,
        target_server_id=server_id.getID(),
        partners=list(partners or []),
    )


def _records_for(topology, spec):
    if hasattr(topology, "records_for"):
        return topology.records_for(spec)
    for device in getattr(topology, "devices", []):
        if device.getID().getID() == spec.device_id().getID():
            return list(device.records)
    return []


def _partner_reference_count(topology, controller_spec, partner_spec):
    partner_id_text = partner_spec.device_id().getID()
    return sum(
        1
        for record in _records_for(topology, controller_spec)
        if record.phase == "partner-reference" and record.value == partner_id_text
    )


def _has_partner_reference_after(topology, controller_spec, partner_spec, previous_count):
    return _partner_reference_count(topology, controller_spec, partner_spec) > previous_count


def _parse_controller_once(server, topology, controller_spec):
    shot = server.makeshot(
        single_device_output(
            controller_spec.device_id(),
            channel=0,
            time_ns=1000,
            value=1.0,
        )
    )
    parse_ticket = server.parse(shot)
    wait_for_ticket(parse_ticket, timeout_s=10.0, diagnostics=topology.diagnostics)


def _wait_for_partner_reference(topology, server, controller_spec, partner_spec, timeout_s=5.0, previous_count=0):
    deadline = time.time() + timeout_s

    while True:
        _parse_controller_once(server, topology, controller_spec)
        if _has_partner_reference_after(topology, controller_spec, partner_spec, previous_count):
            return
        if time.time() >= deadline:
            break
        time.sleep(0.25)

    partner_id_text = partner_spec.device_id().getID()
    raise AssertionError(
        "Partner reference was not recorded for {0}; records: {1}\n\nDiagnostics:\n{2}".format(
            partner_id_text,
            _records_for(topology, controller_spec),
            topology.diagnostics(),
        )
    )


def _wait_for_names_in_network(topology, names, timeout_s=8.0):
    expected = set(names)

    def network_text():
        parts = []
        for attr in ("hub", "server_hub", "partner_hub"):
            hub = getattr(topology, attr, None)
            if hub is None:
                continue
            try:
                parts.extend(device_id.getID() for device_id in hub.getDeviceIDs())
            except Exception:
                pass
        try:
            parts.append(topology.print_network())
        except Exception:
            pass
        return "\n".join(parts)

    def found_all():
        network = network_text()
        return all(name in network for name in expected)

    wait_for(
        found_all,
        timeout_s=timeout_s,
        describe=lambda: "network did not contain devices {0}".format(sorted(expected)),
        diagnostics=topology.diagnostics,
    )


class TwoHubInProcessTopology(object):
    def __init__(self, nameservice_address, stipy, stidevicepy, server_spec, controller_spec, partner_spec, order):
        self.nameservice_address = nameservice_address
        self.stipy = stipy
        self.stidevicepy = stidevicepy
        self.server_spec = server_spec
        self.controller_spec = controller_spec
        self.partner_spec = partner_spec
        self.order = order
        self.persistence_root = None
        self.server_hub = None
        self.partner_hub = None
        self.server = None
        self.controller = None
        self.partner = None
        self.devices = []

    @property
    def server_id(self):
        if self.server is not None:
            return self.server.getID()
        return self.server_spec.device_id()

    def __enter__(self):
        self.persistence_root = tempfile.mkdtemp(prefix="sti3-partner-distribution-")
        for spec in (self.server_spec, self.controller_spec, self.partner_spec):
            if spec.persistence_root is None:
                spec.persistence_root = self.persistence_root

        server_hub_id = self.stipy.HubID(
            "Partner Distribution Server Hub {0}".format(self.controller_spec.module),
            "localhost",
            self.controller_spec.module,
        )
        partner_hub_id = self.stipy.HubID(
            "Partner Distribution Peer Hub {0}".format(self.partner_spec.module),
            "localhost",
            self.partner_spec.module,
        )

        self.server_hub = self.stidevicepy.NetworkDeviceHub(server_hub_id, self.nameservice_address)
        self.partner_hub = self.stidevicepy.NetworkDeviceHub(partner_hub_id, self.nameservice_address)

        self.server = SimulatedDevice(self.server_spec)
        self.controller = SimulatedDevice(self.controller_spec)
        self.partner = SimulatedDevice(self.partner_spec)
        self.devices = [self.controller, self.partner]

        self.server_hub.addDevice(self.server)
        self.server_hub.run(False)

        if self.order == "partner-before-controller":
            self.partner_hub.addDevice(self.partner)
            self.partner_hub.run(False)
            self.server_hub.addDevice(self.controller)
        else:
            self.server_hub.addDevice(self.controller)
            self.partner_hub.addDevice(self.partner)
            self.partner_hub.run(False)

        return self

    def __exit__(self, exc_type, exc, tb):
        self.shutdown()

    def shutdown(self):
        for hub in (self.partner_hub, self.server_hub):
            if hub is not None:
                try:
                    hub.shutdown()
                except Exception:
                    pass
                try:
                    hub.disconnect()
                except Exception:
                    pass
        self.partner_hub = None
        self.server_hub = None
        self.server = None
        self.controller = None
        self.partner = None
        self.devices = []
        if self.persistence_root is not None:
            shutil.rmtree(self.persistence_root, ignore_errors=True)
            self.persistence_root = None

    def connect_stipy(self):
        return self.stipy.connect(self.server_id, self.nameservice_address)

    def print_network(self):
        if self.server_hub is None:
            return "<topology not started>"
        return self.server_hub.printNetwork()

    def diagnostics(self):
        parts = [
            "Server hub network:\n{0}".format(self.server_hub.printNetwork() if self.server_hub is not None else "<none>"),
            "Partner hub network:\n{0}".format(self.partner_hub.printNetwork() if self.partner_hub is not None else "<none>"),
            "Controller records: {0}".format(_records_for(self, self.controller_spec)),
            "Partner records: {0}".format(_records_for(self, self.partner_spec)),
        ]
        return "\n".join(parts)


def test_partner_distribution_control_with_full_partner_id_on_common_hub(sti_nameservice_address, stipy_modules):
    server_spec = make_server_spec(name="Partner Control Server", address="localhost", module=30)
    server_id = server_spec.device_id()
    partner_spec = _make_child_spec("Partner Control Peer", 31, server_id)
    controller_spec = _make_child_spec("Partner Control Controller", 32, server_id, partners=[partner_spec.device_id()])

    with InProcessTopology(sti_nameservice_address, server_spec, [controller_spec, partner_spec]) as topology:
        _wait_for_names_in_network(topology, [controller_spec.name, partner_spec.name], timeout_s=5.0)
        server = topology.connect_stipy()
        assert server is not None, topology.summary()
        _wait_for_partner_reference(topology, server, controller_spec, partner_spec, timeout_s=3.0)


@pytest.mark.parametrize("order", ["partner-before-controller", "controller-before-partner"])
def test_partial_partner_id_distribution_on_common_hub_orderings(sti_nameservice_address, stipy_modules, order):
    stipy, _ = stipy_modules
    server_spec = make_server_spec(name="Partner Common Hub Server {0}".format(order), address="localhost", module=40)
    server_id = server_spec.device_id()
    partner_spec = _make_child_spec("Partner Common Hub Peer {0}".format(order), 41, server_id)
    controller_spec = _make_child_spec(
        "Partner Common Hub Controller {0}".format(order),
        42,
        server_id,
        partners=[_device_id_without_target(stipy, partner_spec)],
    )
    device_specs = [partner_spec, controller_spec] if order == "partner-before-controller" else [controller_spec, partner_spec]

    with InProcessTopology(sti_nameservice_address, server_spec, device_specs) as topology:
        _wait_for_names_in_network(topology, [controller_spec.name, partner_spec.name], timeout_s=5.0)
        server = topology.connect_stipy()
        assert server is not None, topology.summary()
        _wait_for_partner_reference(topology, server, controller_spec, partner_spec, timeout_s=3.0)


def test_partial_partner_id_distribution_after_partner_process_rejoins(sti_nameservice_address, stipy_modules):
    stipy, _ = stipy_modules
    server_spec = make_server_spec(name="Partner Rejoin Server", address="localhost", module=50)
    server_id = server_spec.device_id()
    partner_spec = _make_child_spec("Partner Rejoin Peer", 51, server_id)
    controller_spec = _make_child_spec(
        "Partner Rejoin Controller",
        52,
        server_id,
        partners=[_device_id_without_target(stipy, partner_spec)],
    )

    with ProcessTopology(sti_nameservice_address, server_spec, [controller_spec, partner_spec]) as topology:
        _wait_for_names_in_network(topology, [controller_spec.name, partner_spec.name], timeout_s=12.0)
        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        _wait_for_partner_reference(topology, server, controller_spec, partner_spec, timeout_s=8.0)
        reference_count = _partner_reference_count(topology, controller_spec, partner_spec)

        assert topology.stop_device(partner_spec)
        time.sleep(0.5)
        topology.start_device(partner_spec)

        _wait_for_names_in_network(topology, [controller_spec.name, partner_spec.name], timeout_s=12.0)
        _wait_for_partner_reference(
            topology,
            server,
            controller_spec,
            partner_spec,
            timeout_s=8.0,
            previous_count=reference_count,
        )


@pytest.mark.parametrize("order", ["partner-before-controller", "controller-before-partner"])
def test_partial_partner_id_distribution_across_two_hubs_same_process(
    sti_nameservice_address,
    stipy_modules,
    order,
):
    stipy, stidevicepy = stipy_modules
    server_spec = make_server_spec(name="Partner Two Hub Server {0}".format(order), address="localhost", module=60)
    server_id = server_spec.device_id()
    partner_spec = _make_child_spec("Partner Two Hub Peer {0}".format(order), 61, server_id)
    controller_spec = _make_child_spec(
        "Partner Two Hub Controller {0}".format(order),
        62,
        server_id,
        partners=[_device_id_without_target(stipy, partner_spec)],
    )

    with TwoHubInProcessTopology(
        sti_nameservice_address,
        stipy,
        stidevicepy,
        server_spec,
        controller_spec,
        partner_spec,
        order,
    ) as topology:
        _wait_for_names_in_network(topology, [controller_spec.name, partner_spec.name], timeout_s=12.0)
        server = topology.connect_stipy()
        assert server is not None, topology.diagnostics()
        _wait_for_partner_reference(topology, server, controller_spec, partner_spec, timeout_s=8.0)


@pytest.mark.parametrize(
    "order",
    ["partner-before-controller", "controller-before-partner"],
)
def test_partial_partner_id_distribution_across_process_hubs(sti_nameservice_address, stipy_modules, order):
    stipy, _ = stipy_modules
    server_spec = make_server_spec(name="Partner Process Server {0}".format(order), address="localhost", module=70)
    server_id = server_spec.device_id()
    partner_spec = _make_child_spec("Partner Process Peer {0}".format(order), 71, server_id)
    controller_spec = _make_child_spec(
        "Partner Process Controller {0}".format(order),
        72,
        server_id,
        partners=[_device_id_without_target(stipy, partner_spec)],
    )
    device_specs = [partner_spec, controller_spec] if order == "partner-before-controller" else [controller_spec, partner_spec]

    with ProcessTopology(sti_nameservice_address, server_spec, device_specs) as topology:
        _wait_for_names_in_network(topology, [controller_spec.name, partner_spec.name], timeout_s=12.0)
        server = topology.connect_stipy()
        assert server is not None, topology.summary()
        _wait_for_partner_reference(topology, server, controller_spec, partner_spec, timeout_s=8.0)
