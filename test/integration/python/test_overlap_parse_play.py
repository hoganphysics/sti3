"""Overlapping parse/play integration scenarios."""

import pytest

from sti_testnet.devices import DeviceBehavior
from sti_testnet.devices import make_device_spec
from sti_testnet.devices import make_server_spec
from sti_testnet.shots import single_device_output
from sti_testnet.topology import InProcessTopology
from sti_testnet.waits import wait_for
from sti_testnet.waits import wait_for_device_ids
from sti_testnet.waits import wait_for_ticket


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice, pytest.mark.slow]


@pytest.mark.xfail(reason="overlap parse/play can play devices but leave result tickets canceled", strict=False)
def test_parse_second_shot_while_first_shot_is_playing(sti_nameservice_address, stipy_modules):
    server_spec = make_server_spec(name="Overlap Server", address="localhost", module=30)
    server_id = server_spec.device_id()
    slow_device_spec = make_device_spec(
        name="Overlap Slow Device",
        address="localhost",
        module=31,
        server_id=server_id,
        behavior=DeviceBehavior(play_delay_s=0.5),
    )
    next_device_spec = make_device_spec(
        name="Overlap Next Device",
        address="localhost",
        module=32,
        server_id=server_id,
    )

    with InProcessTopology(sti_nameservice_address, server_spec, [slow_device_spec, next_device_spec]) as topology:
        wait_for_device_ids(
            topology.hub,
            [server_id, slow_device_spec.device_id(), next_device_spec.device_id()],
            timeout_s=5.0,
            diagnostics=topology.diagnostics,
        )

        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        first_shot = server.makeshot(
            single_device_output(slow_device_spec.device_id(), channel=0, time_ns=1000, value=4.0)
        )
        first_parse = server.parse(first_shot)
        wait_for_ticket(first_parse, timeout_s=10.0, diagnostics=topology.diagnostics)

        first_result = server.play(first_parse)
        slow_device = topology.devices[0]
        wait_for(
            lambda: slow_device.records_for("load"),
            timeout_s=3.0,
            describe=lambda: "first shot did not begin loading on the slow device",
            diagnostics=topology.diagnostics,
        )

        second_shot = server.makeshot(
            single_device_output(next_device_spec.device_id(), channel=0, time_ns=2000, value=5.0)
        )
        second_parse = server.parse(second_shot)
        wait_for_ticket(second_parse, timeout_s=10.0, diagnostics=topology.diagnostics)

        wait_for_ticket(first_result, timeout_s=10.0, diagnostics=topology.diagnostics)

        second_result = server.play(second_parse)
        wait_for_ticket(second_result, timeout_s=10.0, diagnostics=topology.diagnostics)

        next_device = topology.devices[1]
        assert slow_device.records_for("play")
        assert next_device.records_for("play")
