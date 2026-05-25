"""Harness smoke tests for simulated STI networks."""

import pytest

from sti_testnet.devices import make_device_spec
from sti_testnet.devices import make_server_spec
from sti_testnet.shots import single_device_output
from sti_testnet.topology import InProcessTopology
from sti_testnet.waits import wait_for_device_ids
from sti_testnet.waits import wait_for_ticket


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice]


def test_in_process_single_device_parse_play_smoke(sti_nameservice_address, stipy_modules):
    server_spec = make_server_spec(name="Integration Test Server", address="localhost", module=0)
    server_id = server_spec.device_id()
    device_spec = make_device_spec(
        name="Integration Test Device",
        address="localhost",
        module=1,
        server_id=server_id,
    )

    with InProcessTopology(sti_nameservice_address, server_spec, [device_spec]) as topology:
        wait_for_device_ids(topology.hub, [server_id, device_spec.device_id()], timeout_s=5.0, diagnostics=topology.diagnostics)

        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        shot = server.makeshot(single_device_output(device_spec.device_id(), channel=0, time_ns=1000, value=2.5))
        parse_ticket = server.parse(shot)
        wait_for_ticket(parse_ticket, timeout_s=10.0, diagnostics=topology.diagnostics)

        result_ticket = server.play(parse_ticket)
        wait_for_ticket(result_ticket, timeout_s=10.0, diagnostics=topology.diagnostics)

        device = topology.devices[0]
        assert device.records_for("load")
        assert device.records_for("play")
