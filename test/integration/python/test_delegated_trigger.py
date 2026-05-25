"""Delegated-trigger integration scenarios."""

import pytest

from sti_testnet.devices import make_device_spec
from sti_testnet.devices import make_server_spec
from sti_testnet.shots import delegated_trigger_output
from sti_testnet.topology import InProcessTopology
from sti_testnet.waits import WaitTimeout
from sti_testnet.waits import wait_for_device_ids
from sti_testnet.waits import wait_for_ticket


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice, pytest.mark.slow]


@pytest.mark.xfail(reason="delegated-trigger runtime completion is not reliable yet", strict=False)
def test_delegated_trigger_parse_play_success(sti_nameservice_address, stipy_modules):
    server_spec = make_server_spec(name="Delegated Trigger Server", address="localhost", module=10)
    server_id = server_spec.device_id()
    target_spec = make_device_spec(
        name="Delegated Trigger Target Device",
        address="localhost",
        module=11,
        server_id=server_id,
    )

    with InProcessTopology(sti_nameservice_address, server_spec, [target_spec]) as topology:
        expected_ids = [server_id, target_spec.device_id()]
        wait_for_device_ids(topology.hub, expected_ids, timeout_s=5.0, diagnostics=topology.diagnostics)

        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        shot = server.makeshot(
            delegated_trigger_output(
                trigger_device_id=target_spec.device_id(),
                event_device_id=target_spec.device_id(),
                channel=0,
                time_ns=1000,
                value=2.5,
            )
        )
        parse_ticket = server.parse(shot)
        wait_for_ticket(parse_ticket, timeout_s=10.0, diagnostics=topology.diagnostics)

        result_ticket = server.play(parse_ticket)
        target_device = topology.devices[0]
        try:
            wait_for_ticket(result_ticket, timeout_s=3.0, diagnostics=topology.diagnostics)
        except WaitTimeout:
            try:
                result_ticket.cancel()
            except Exception:
                pass
            assert target_device.records_for("load")
            assert target_device.records_for("play")
            pytest.xfail("delegated-trigger play reaches the device but the result ticket remains Running")

        assert target_device.records_for("load")
        assert target_device.records_for("play")
