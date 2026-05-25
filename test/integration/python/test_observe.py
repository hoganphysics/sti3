"""Opt-in observable topologies for frontend inspection."""

import time

import pytest

from sti_testnet.devices import make_device_spec
from sti_testnet.devices import make_server_spec
from sti_testnet.topology import InProcessTopology
from sti_testnet.waits import wait_for_device_ids


pytestmark = [pytest.mark.observe, pytest.mark.requires_nameservice]


def test_observe_basic_in_process_network(external_sti_nameservice, observe_timeout, stipy_modules):
    server_spec = make_server_spec(name="Observe Test Server", address="localhost", module=0)
    server_id = server_spec.device_id()
    device_specs = [
        make_device_spec("Observe Device {0}".format(index), server_id, module=index)
        for index in range(1, 4)
    ]

    with InProcessTopology(external_sti_nameservice, server_spec, device_specs) as topology:
        wait_for_device_ids(
            topology.hub,
            [server_id] + [spec.device_id() for spec in device_specs],
            timeout_s=5.0,
            diagnostics=topology.diagnostics,
        )
        print("\n" + topology.summary())
        time.sleep(observe_timeout)
