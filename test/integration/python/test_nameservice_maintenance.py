"""NameService maintenance regression tests."""

import time

import pytest

from sti_testnet.devices import make_device_spec
from sti_testnet.devices import make_server_spec
from sti_testnet.topology import ProcessTopology
from sti_testnet.waits import wait_for


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice]


def _server_hub_context(topology):
    return "STI/{0}".format(topology.hub.getID().getID().replace(".", "_"))


def _server_subtree(topology):
    return topology.hub.printNetwork(_server_hub_context(topology))


def _hub_object_count(tree):
    return tree.count("TDeviceHub.Object")


def _prune_config(stipy):
    config = stipy.Configuration()
    config.set("NetworkHub", "EnablePrune", "true")
    config.set("NetworkHub", "PruneScope", "OwnIncomingSubtree")
    config.set("NetworkHub", "PruneIntervalSeconds", "1")
    config.set("NetworkHub", "PruneFailureThreshold", "2")
    config.set("NetworkHub", "PruneSuspectSeconds", "1")
    return config


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
            lambda: _hub_object_count(_server_subtree(topology)) >= 2,
            timeout_s=10.0,
            describe=lambda: "server and device object bindings did not appear under server context",
            diagnostics=topology.diagnostics,
        )

        assert topology.stop_device(device_spec), topology.diagnostics()

        # The server refresh task runs every five seconds. A stopped child process
        # leaves a stale object reference, but normal discovery must not prune it.
        time.sleep(6.0)

        assert _hub_object_count(_server_subtree(topology)) >= 2, topology.diagnostics()


def test_enabled_prune_removes_stale_process_hub_child_binding_after_threshold(
    sti_nameservice_address,
    stipy_modules,
):
    stipy, _ = stipy_modules
    server_spec = make_server_spec(name="Nameservice Prune Server", address="localhost", module=43)
    server_id = server_spec.device_id()
    device_spec = make_device_spec(
        name="Nameservice Prune Device",
        address="localhost",
        module=44,
        server_id=server_id,
    )

    with ProcessTopology(
        sti_nameservice_address,
        server_spec,
        [device_spec],
        hub_config=_prune_config(stipy),
    ) as topology:
        wait_for(
            lambda: _hub_object_count(_server_subtree(topology)) >= 2,
            timeout_s=10.0,
            describe=lambda: "server and device object bindings did not appear under server context",
            diagnostics=topology.diagnostics,
        )

        assert topology.stop_device(device_spec), topology.diagnostics()

        wait_for(
            lambda: _hub_object_count(_server_subtree(topology)) == 1,
            timeout_s=8.0,
            describe=lambda: "stale device object binding was not pruned under server context",
            diagnostics=topology.diagnostics,
        )
