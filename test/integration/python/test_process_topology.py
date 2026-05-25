"""Process-isolated simulated-device topology tests."""

import pytest

from sti_testnet.devices import make_device_spec
from sti_testnet.devices import make_server_spec
from sti_testnet.shots import single_device_output
from sti_testnet.topology import ProcessTopology
from sti_testnet.waits import wait_for
from sti_testnet.waits import wait_for_ticket


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice]


def test_process_topology_single_device_parse_play(sti_nameservice_address, stipy_modules):
    server_spec = make_server_spec(name="Process Topology Server", address="localhost", module=20)
    server_id = server_spec.device_id()
    device_spec = make_device_spec(
        name="Process Topology Device",
        address="localhost",
        module=21,
        server_id=server_id,
    )

    with ProcessTopology(sti_nameservice_address, server_spec, [device_spec]) as topology:
        wait_for(
            lambda: device_spec.name in topology.print_network(),
            timeout_s=10.0,
            describe=lambda: "process device did not appear in server hub",
            diagnostics=topology.diagnostics,
        )

        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        shot = server.makeshot(single_device_output(device_spec.device_id(), channel=0, time_ns=1000, value=3.5))
        parse_ticket = server.parse(shot)
        wait_for_ticket(parse_ticket, timeout_s=10.0, diagnostics=topology.diagnostics)

        result_ticket = server.play(parse_ticket)
        wait_for_ticket(result_ticket, timeout_s=10.0, diagnostics=topology.diagnostics)

        records = topology.records_for(device_spec)
        assert [record for record in records if record.phase == "load"]
        assert [record for record in records if record.phase == "play"]
