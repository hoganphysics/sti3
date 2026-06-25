"""NameService maintenance regression tests."""

import time

import pytest

from sti_testnet.devices import make_device_spec
from sti_testnet.devices import make_server_spec
from sti_testnet.topology import ProcessTopology
from sti_testnet.waits import wait_for


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice]


def test_stale_process_hub_binding_survives_periodic_discovery(sti_nameservice_address, stipy_modules):
    server_spec = make_server_spec(name="Nameservice Maintenance Server", address="localhost", module=41)
    server_id = server_spec.device_id()
    device_spec = make_device_spec(
        name="Nameservice Maintenance Device",
        address="localhost",
        module=42,
        server_id=server_id,
    )

    with ProcessTopology(sti_nameservice_address, server_spec, [device_spec]) as topology:
        wait_for(
            lambda: topology.print_network().count(device_spec.name) >= 2,
            timeout_s=10.0,
            describe=lambda: "device root and server child bindings did not appear",
            diagnostics=topology.diagnostics,
        )

        assert topology.stop_device(device_spec), topology.diagnostics()

        # The server refresh task runs every five seconds. A stopped child process
        # leaves a stale object reference, but normal discovery must not prune it.
        time.sleep(6.0)

        network = topology.print_network()
        assert network.count(device_spec.name) >= 2, topology.diagnostics()
