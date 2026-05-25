"""Network stress integration scenarios."""

import pytest

from sti_testnet.devices import make_device_spec
from sti_testnet.devices import make_server_spec
from sti_testnet.shots import generated_output_events
from sti_testnet.topology import InProcessTopology
from sti_testnet.waits import wait_for_device_ids
from sti_testnet.waits import wait_for_ticket


pytestmark = [pytest.mark.integration, pytest.mark.stress, pytest.mark.requires_nameservice]


def test_generated_device_stress_smoke(sti_nameservice_address, stipy_modules):
    device_count = 6
    events_per_device = 3
    server_spec = make_server_spec(name="Stress Smoke Server", address="localhost", module=40)
    server_id = server_spec.device_id()
    device_specs = [
        make_device_spec(
            name="Stress Smoke Device {0}".format(index),
            address="localhost",
            module=41 + index,
            server_id=server_id,
        )
        for index in range(device_count)
    ]

    with InProcessTopology(sti_nameservice_address, server_spec, device_specs) as topology:
        wait_for_device_ids(
            topology.hub,
            [server_id] + [spec.device_id() for spec in device_specs],
            timeout_s=10.0,
            diagnostics=topology.diagnostics,
        )

        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        shot = server.makeshot(
            generated_output_events(
                [spec.device_id() for spec in device_specs],
                events_per_device=events_per_device,
                first_time_ns=1000,
                spacing_ns=1000,
                channel=0,
                value=7.0,
            )
        )
        parse_ticket = server.parse(shot)
        wait_for_ticket(parse_ticket, timeout_s=20.0, diagnostics=topology.diagnostics)

        result_ticket = server.play(parse_ticket)
        wait_for_ticket(result_ticket, timeout_s=20.0, diagnostics=topology.diagnostics)

        for device in topology.devices:
            assert len(device.records_for("load")) == events_per_device
            assert len(device.records_for("play")) == events_per_device
