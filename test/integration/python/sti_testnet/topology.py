"""Topology builders and lifecycle handles for simulated STI networks."""

import atexit
import gc
import json
import os
import select
import shutil
import subprocess
import sys
import tempfile
import time

from .devices import event_record_from_dict
from .devices import SimulatedDevice
from .devices import make_server_spec
from .devices import require_stipy


_OWNED_PERSISTENCE_ROOTS = set()


def cleanup_registered_persistence_roots():
    for root in list(_OWNED_PERSISTENCE_ROOTS):
        shutil.rmtree(root, ignore_errors=True)
        if not _path_exists(root):
            _OWNED_PERSISTENCE_ROOTS.discard(root)


atexit.register(cleanup_registered_persistence_roots)


def _path_exists(path):
    try:
        import os
        return os.path.exists(path)
    except Exception:
        return True


class FrontendConnectionInfo(object):
    def __init__(self, nameservice_address, server_id):
        self.nameservice_address = nameservice_address
        self.server_id = server_id

    @property
    def nameservice_host(self):
        if ":" in self.nameservice_address:
            return self.nameservice_address.rsplit(":", 1)[0]
        return self.nameservice_address

    @property
    def nameservice_port(self):
        if ":" in self.nameservice_address:
            return self.nameservice_address.rsplit(":", 1)[1]
        return ""

    def lines(self):
        server_id = self.server_id
        return [
            "Name service: {0}".format(self.nameservice_address),
            "Name service host: {0}".format(self.nameservice_host),
            "Name service port: {0}".format(self.nameservice_port),
            "Server DeviceID: {0}".format(server_id.getID()),
            "Server name: {0}".format(server_id.name()),
            "Server address: {0}".format(server_id.address()),
            "Server module: {0}".format(server_id.module()),
        ]

    def __str__(self):
        return "\n".join(self.lines())


class InProcessTopology(object):
    def __init__(self, nameservice_address, server_spec=None, device_specs=None, hub_id=None, persistence_root=None):
        self.nameservice_address = nameservice_address
        self.server_spec = server_spec or make_server_spec()
        self.device_specs = list(device_specs or [])
        self.hub_id = hub_id
        self.persistence_root = persistence_root
        self._owns_persistence_root = persistence_root is None
        self.hub = None
        self.server = None
        self.devices = []
        self.started = False

    @property
    def server_id(self):
        if self.server is not None:
            return self.server.getID()
        return self.server_spec.device_id()

    @property
    def frontend(self):
        return FrontendConnectionInfo(self.nameservice_address, self.server_id)

    def start(self):
        _, stidevicepy = require_stipy()
        if self.persistence_root is None:
            self.persistence_root = tempfile.mkdtemp(prefix="sti3-integration-")
            _OWNED_PERSISTENCE_ROOTS.add(self.persistence_root)
        self._apply_persistence_root()

        if self.hub_id is None:
            self.hub = stidevicepy.NetworkDeviceHub(self.nameservice_address)
        else:
            self.hub = stidevicepy.NetworkDeviceHub(self.hub_id, self.nameservice_address)

        self.server = SimulatedDevice(self.server_spec)
        self.hub.addDevice(self.server)
        self.hub.run(False)

        self.devices = []
        for spec in self.device_specs:
            device = SimulatedDevice(spec)
            self.devices.append(device)
            self.hub.addDevice(device)

        self.started = True
        return self

    def shutdown(self):
        if self.hub is not None:
            try:
                self.hub.shutdown()
            except Exception:
                pass
            try:
                self.hub.disconnect()
            except Exception:
                pass
        self.hub = None
        self.server = None
        self.devices = []
        gc.collect()
        if self._owns_persistence_root and self.persistence_root is not None:
            shutil.rmtree(self.persistence_root, ignore_errors=True)
            self.persistence_root = None
        self.started = False

    def __enter__(self):
        return self.start()

    def __exit__(self, exc_type, exc, tb):
        self.shutdown()

    def connect_stipy(self):
        stipy, _ = require_stipy()
        return stipy.connect(self.server_id, self.nameservice_address)

    def print_network(self):
        if self.hub is None:
            return "<topology not started>"
        return self.hub.printNetwork()

    def summary(self):
        lines = self.frontend.lines()
        lines.append("Persistence root: {0}".format(self.persistence_root))
        lines.append("Known devices:")
        if self.server is not None:
            lines.append("  {0}".format(self.server.getID().getID()))
        for device in self.devices:
            lines.append("  {0}".format(device.getID().getID()))
        return "\n".join(lines)

    def diagnostics(self):
        lines = [self.summary()]
        if self.hub is not None:
            try:
                lines.append("Hub DeviceIDs: {0}".format([device_id.getID() for device_id in self.hub.getDeviceIDs()]))
            except Exception as exc:
                lines.append("Hub DeviceIDs unavailable: {0}".format(exc))
            try:
                lines.append("Network:\n{0}".format(self.hub.printNetwork()))
            except Exception as exc:
                lines.append("Network summary unavailable: {0}".format(exc))
        for device in self.devices:
            lines.append("Records for {0}: {1}".format(device.getID().getID(), device.records))
        return "\n".join(lines)

    def _apply_persistence_root(self):
        specs = [self.server_spec] + self.device_specs
        for spec in specs:
            if spec.persistence_root is None:
                spec.persistence_root = self.persistence_root


class ProcessTopology(object):
    def __init__(self, nameservice_address, server_spec=None, device_specs=None, persistence_root=None, ready_timeout_s=10.0):
        self.nameservice_address = nameservice_address
        self.server_spec = server_spec or make_server_spec()
        self.device_specs = list(device_specs or [])
        self.persistence_root = persistence_root
        self._owns_persistence_root = persistence_root is None
        self.ready_timeout_s = float(ready_timeout_s)
        self.hub = None
        self.server = None
        self.device_processes = []
        self.started = False

    @property
    def server_id(self):
        if self.server is not None:
            return self.server.getID()
        return self.server_spec.device_id()

    @property
    def frontend(self):
        return FrontendConnectionInfo(self.nameservice_address, self.server_id)

    def start(self):
        _, stidevicepy = require_stipy()
        if self.persistence_root is None:
            self.persistence_root = tempfile.mkdtemp(prefix="sti3-integration-")
            _OWNED_PERSISTENCE_ROOTS.add(self.persistence_root)
        self._apply_persistence_root()

        self.hub = stidevicepy.NetworkDeviceHub(self.nameservice_address)
        self.server = SimulatedDevice(self.server_spec)
        self.hub.addDevice(self.server)
        self.hub.run(False)

        for index, spec in enumerate(self.device_specs):
            self.device_processes.append(self._start_device_process(index, spec))

        self.started = True
        return self

    def shutdown(self):
        for handle in list(self.device_processes):
            handle.shutdown()
        self.device_processes = []
        if self.hub is not None:
            try:
                self.hub.shutdown()
            except Exception:
                pass
            try:
                self.hub.disconnect()
            except Exception:
                pass
        self.hub = None
        self.server = None
        gc.collect()
        if self._owns_persistence_root and self.persistence_root is not None:
            shutil.rmtree(self.persistence_root, ignore_errors=True)
            self.persistence_root = None
        self.started = False

    def __enter__(self):
        return self.start()

    def __exit__(self, exc_type, exc, tb):
        self.shutdown()

    def connect_stipy(self):
        stipy, _ = require_stipy()
        return stipy.connect(self.server_id, self.nameservice_address)

    def print_network(self):
        if self.hub is None:
            return "<topology not started>"
        return self.hub.printNetwork()

    def summary(self):
        lines = self.frontend.lines()
        lines.append("Persistence root: {0}".format(self.persistence_root))
        lines.append("Known devices:")
        if self.server is not None:
            lines.append("  {0}".format(self.server.getID().getID()))
        for handle in self.device_processes:
            lines.append("  {0} pid={1}".format(handle.device_id_text, handle.pid))
        return "\n".join(lines)

    def diagnostics(self):
        lines = [self.summary()]
        if self.hub is not None:
            try:
                self.hub.refresh()
            except Exception:
                pass
            try:
                lines.append("Hub DeviceIDs: {0}".format([device_id.getID() for device_id in self.hub.getDeviceIDs()]))
            except Exception as exc:
                lines.append("Hub DeviceIDs unavailable: {0}".format(exc))
            try:
                lines.append("Network:\n{0}".format(self.hub.printNetwork()))
            except Exception as exc:
                lines.append("Network summary unavailable: {0}".format(exc))
        for handle in self.device_processes:
            lines.append(handle.diagnostics())
        return "\n".join(lines)

    def records_for(self, spec_or_id):
        device_id_text = _device_id_text(spec_or_id)
        for handle in self.device_processes:
            if handle.device_id_text == device_id_text:
                return handle.records()
        return []

    def stop_device(self, spec_or_id):
        device_id_text = _device_id_text(spec_or_id)
        for handle in list(self.device_processes):
            if handle.device_id_text == device_id_text:
                handle.shutdown()
                self.device_processes.remove(handle)
                return True
        return False

    def start_device(self, spec):
        handle = self._start_device_process(len(self.device_processes), spec)
        self.device_processes.append(handle)
        return handle

    def _apply_persistence_root(self):
        specs = [self.server_spec] + self.device_specs
        for spec in specs:
            if spec.persistence_root is None:
                spec.persistence_root = self.persistence_root

    def _start_device_process(self, index, spec):
        spec_dir = os.path.join(self.persistence_root, "process-specs")
        record_dir = os.path.join(self.persistence_root, "process-records")
        os.makedirs(spec_dir, exist_ok=True)
        os.makedirs(record_dir, exist_ok=True)

        if spec.record_path is None:
            spec.record_path = os.path.join(record_dir, "{0}.jsonl".format(_safe_filename(spec.name)))

        spec_path = os.path.join(spec_dir, "device-{0}.json".format(index))
        with open(spec_path, "w") as handle:
            json.dump(spec.to_dict(), handle, sort_keys=True)

        process = subprocess.Popen(
            [
                sys.executable,
                "-m",
                "sti_testnet.device_process",
                "--nameservice",
                self.nameservice_address,
                "--spec",
                spec_path,
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            close_fds=True,
        )
        handle = _DeviceProcessHandle(process, spec, spec_path, self.ready_timeout_s)
        try:
            handle.wait_until_ready()
        except Exception:
            handle.shutdown()
            raise
        return handle


class _DeviceProcessHandle(object):
    def __init__(self, process, spec, spec_path, ready_timeout_s):
        self.process = process
        self.spec = spec
        self.spec_path = spec_path
        self.ready_timeout_s = ready_timeout_s
        self.stdout_lines = []
        self.stderr_text = ""

    @property
    def pid(self):
        return self.process.pid

    @property
    def device_id_text(self):
        return self.spec.device_id().getID()

    def wait_until_ready(self):
        deadline = time.time() + self.ready_timeout_s
        while time.time() < deadline:
            if self.process.poll() is not None:
                self._collect_output()
                raise RuntimeError("device process exited before ready:\n{0}".format(self.diagnostics()))
            readable, _, _ = select.select([self.process.stdout], [], [], 0.1)
            if not readable:
                continue
            line = self.process.stdout.readline()
            if line:
                line = line.rstrip()
                self.stdout_lines.append(line)
                if line.startswith("STI_TESTNET_DEVICE_READY "):
                    return
        raise RuntimeError("timed out waiting for device process:\n{0}".format(self.diagnostics()))

    def shutdown(self):
        if self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=2.0)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait(timeout=2.0)
        self._collect_output()

    def records(self):
        if self.spec.record_path is None or not os.path.exists(self.spec.record_path):
            return []
        records = []
        with open(self.spec.record_path, "r") as handle:
            for line in handle:
                line = line.strip()
                if line:
                    records.append(event_record_from_dict(json.loads(line)))
        return records

    def diagnostics(self):
        parts = [
            "Device process {0} pid={1} returncode={2}".format(
                self.device_id_text,
                self.pid,
                self.process.poll(),
            ),
            "Spec path: {0}".format(self.spec_path),
            "Record path: {0}".format(self.spec.record_path),
            "Records: {0}".format(self.records()),
        ]
        if self.stdout_lines:
            parts.append("stdout:\n{0}".format("\n".join(self.stdout_lines)))
        if self.stderr_text:
            parts.append("stderr:\n{0}".format(self.stderr_text))
        return "\n".join(parts)

    def _collect_output(self):
        if self.process.stdout is not None:
            for line in self.process.stdout.readlines():
                self.stdout_lines.append(line.rstrip())
        if self.process.stderr is not None:
            self.stderr_text += self.process.stderr.read()


def _safe_filename(text):
    safe = []
    for char in text:
        if char.isalnum() or char in ("-", "_"):
            safe.append(char)
        else:
            safe.append("_")
    return "".join(safe) or "device"


def _device_id_text(spec_or_id):
    if hasattr(spec_or_id, "device_id"):
        return spec_or_id.device_id().getID()
    if hasattr(spec_or_id, "getID"):
        return spec_or_id.getID()
    return str(spec_or_id)
