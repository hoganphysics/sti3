"""NameService maintenance regression tests."""

from pathlib import Path
import time

import pytest

from sti_testnet.devices import make_device_spec
from sti_testnet.devices import make_server_spec
from sti_testnet.topology import ProcessTopology
from sti_testnet.waits import wait_for


pytestmark = [pytest.mark.integration, pytest.mark.requires_nameservice]


def _server_hub_context(topology):
    server_id = topology.server_id
    address = server_id.address().replace(".", "_").replace("/", "_")
    name = server_id.name().replace(".", "_").replace("/", "_")
    return "STI/{0}/{1}/Hub::{2}".format(address, server_id.module(), name)


def _server_subtree(topology):
    return topology.hub.printNetwork(_server_hub_context(topology))


def _hub_object_count(tree):
    return tree.count("TDeviceHub.Object")


def _has_device_names(tree, specs):
    return all(spec.name in tree for spec in specs)


def _server_process_saw_add(topology, spec):
    if topology.server_process is None:
        return False
    return topology.server_process.stdout_contains("++++ add( {0} )".format(spec.device_id().getID()))


def _prune_config(stipy):
    config = stipy.Configuration()
    config.set("NetworkHub", "EnablePrune", "true")
    config.set("NetworkHub", "PruneScope", "OwnIncomingSubtree")
    config.set("NetworkHub", "PruneIntervalSeconds", "1")
    config.set("NetworkHub", "PruneFailureThreshold", "2")
    config.set("NetworkHub", "PruneSuspectSeconds", "1")
    return config


def _self_rebind_config(stipy):
    config = stipy.Configuration()
    config.set("NetworkHub", "SelfRebind", "true")
    config.set("NetworkHub", "SelfRebindIntervalSeconds", "0.1")
    config.set("NetworkHub", "EnablePrune", "false")
    return config


def _nameservice_data_size(service):
    return sum(path.stat().st_size for path in Path(service.datadir).glob("*.dat"))


def test_unchanged_periodic_self_rebind_does_not_grow_nameservice_data(
    sti_nameservice_address,
    spawned_sti_nameservice,
    stipy_modules,
):
    if spawned_sti_nameservice is None:
        pytest.skip("test requires a spawned omniNames data directory")

    stipy, _ = stipy_modules
    server_spec = make_server_spec(name="Idempotent Rebind Server", address="localhost", module=40)

    with ProcessTopology(
        sti_nameservice_address,
        server_spec,
        [],
        hub_config=_self_rebind_config(stipy),
    ) as topology:
        wait_for(
            lambda: _hub_object_count(_server_subtree(topology)) == 1,
            timeout_s=5.0,
            describe=lambda: "server object binding did not appear",
            diagnostics=topology.diagnostics,
        )

        # Allow several self-rebind intervals after the initial registration,
        # then verify additional unchanged intervals do not append redo records.
        time.sleep(0.5)
        size_before = _nameservice_data_size(spawned_sti_nameservice)
        time.sleep(0.5)
        size_after = _nameservice_data_size(spawned_sti_nameservice)

        assert size_before > 0, topology.diagnostics()
        assert size_after <= size_before, topology.diagnostics()


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


def test_server_restart_recovers_live_process_hubs_with_stale_child_binding(
    sti_nameservice_address,
    stipy_modules,
):
    server_spec = make_server_spec(name="Nameservice Restart Server", address="localhost", module=45)
    server_id = server_spec.device_id()
    live_specs = [
        make_device_spec(
            name="Nameservice Restart Device A",
            address="localhost",
            module=46,
            server_id=server_id,
        ),
        make_device_spec(
            name="Nameservice Restart Device B",
            address="localhost",
            module=47,
            server_id=server_id,
        ),
    ]
    stale_spec = make_device_spec(
        name="Nameservice Restart Stale Device",
        address="localhost",
        module=48,
        server_id=server_id,
    )
    device_specs = live_specs + [stale_spec]

    with ProcessTopology(
        sti_nameservice_address,
        server_spec,
        device_specs,
        server_in_process=False,
    ) as topology:
        wait_for(
            lambda: _has_device_names(topology.print_network(), device_specs),
            timeout_s=12.0,
            describe=lambda: "initial process device bindings did not reach nameservice",
            diagnostics=topology.diagnostics,
        )
        wait_for(
            lambda: _hub_object_count(_server_subtree(topology)) >= 4,
            timeout_s=10.0,
            describe=lambda: "server and process device object bindings did not appear under server context",
            diagnostics=topology.diagnostics,
        )

        assert topology.stop_device(stale_spec), topology.diagnostics()

        assert _hub_object_count(_server_subtree(topology)) >= 4, topology.diagnostics()

        topology.restart_server_hub()

        wait_for(
            lambda: all(_server_process_saw_add(topology, spec) for spec in live_specs),
            timeout_s=12.0,
            describe=lambda: "restarted server process did not re-add all live device references",
            diagnostics=topology.diagnostics,
        )
        assert not _server_process_saw_add(topology, stale_spec), topology.diagnostics()
