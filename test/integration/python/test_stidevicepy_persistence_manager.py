"""stidevicepy persistence manager API coverage."""

from pathlib import Path

import pytest

from sti_testnet.devices import DeviceSpec
from sti_testnet.devices import make_server_spec
from sti_testnet.topology import InProcessTopology
from sti_testnet.waits import wait_for_device_ids


def test_stidevicepy_persistence_manager_exposes_file_server(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules

    config = stipy.Configuration()
    config.set("Device Name", "Persistence Manager FileServer Device")
    config.set("IP Address", "localhost")
    config.set("Module", "54")
    config.set("Target Server", "Persistence Manager FileServer Server")
    config.set("PersistenceManager", "root path", str(tmp_path))
    config.set("PersistenceManager", "device subdirectory", "device")

    device = stidevicepy.LocalDevice(config)
    persistence = device.getPersistenceManager()

    assert persistence is not None
    assert hasattr(persistence, "getFileServer")

    file_server = persistence.getFileServer()
    assert file_server is not None
    assert isinstance(file_server, stipy.FileServer)


def test_stidevicepy_device_upload_file_imports_local_path(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules

    source_path = tmp_path / "source" / "payload.bin"
    source_path.parent.mkdir()
    payload = b"upload_file local payload\x00with-nul"
    source_path.write_bytes(payload)

    config = stipy.Configuration()
    config.set("Device Name", "Device Upload File Device")
    config.set("IP Address", "localhost")
    config.set("Module", "64")
    config.set("Target Server", "Device Upload File Server")
    config.set("PersistenceManager", "root path", str(tmp_path / "persistence"))
    config.set("PersistenceManager", "device subdirectory", "device")

    device = stidevicepy.LocalDevice(config)
    persistence = device.getPersistenceManager()
    file_server = persistence.getFileServer()

    with device.upload_file(source_path) as uploaded:
        imported_id = uploaded.fileID
        imported_path = Path(imported_id.path) / imported_id.filename

        assert file_server.findFile(imported_id)
        assert file_server.getFileSize(imported_id) == len(payload)
        assert imported_path.read_bytes() == payload

    assert uploaded.closed
    assert not file_server.findFile(imported_id)
    assert not imported_path.exists()

    options = stipy.ImportFileOptions(storage=stipy.ImportStorage.Virtual)
    with device.upload_file(source_path, options=options) as uploaded:
        virtual_id = uploaded.fileID
        assert file_server.findFile(virtual_id)
        assert file_server.getFileSize(virtual_id) == len(payload)

    assert uploaded.closed
    assert not file_server.findFile(virtual_id)


def test_stidevicepy_device_upload_file_rejects_missing_path(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules

    config = stipy.Configuration()
    config.set("Device Name", "Device Upload Missing File Device")
    config.set("IP Address", "localhost")
    config.set("Module", "65")
    config.set("Target Server", "Device Upload Missing File Server")
    config.set("PersistenceManager", "root path", str(tmp_path / "persistence"))

    device = stidevicepy.LocalDevice(config)

    with pytest.raises(FileNotFoundError):
        device.upload_file(tmp_path / "missing.bin")


def test_stidevicepy_device_upload_data_imports_bytes_like_payload(
    stipy_modules,
    tmp_path,
    monkeypatch,
):
    stipy, stidevicepy = stipy_modules

    client_path = tmp_path / "client"
    client_path.mkdir()
    monkeypatch.chdir(client_path)

    config = stipy.Configuration()
    config.set("Device Name", "Device Upload Data Device")
    config.set("IP Address", "localhost")
    config.set("Module", "66")
    config.set("Target Server", "Device Upload Data Server")
    config.set("PersistenceManager", "root path", str(tmp_path / "persistence"))
    config.set("PersistenceManager", "device subdirectory", "device")

    device = stidevicepy.LocalDevice(config)
    persistence = device.getPersistenceManager()
    file_server = persistence.getFileServer()
    payload = bytearray(b"upload_data in-memory payload\x00with-nul")

    with device.upload_data(memoryview(payload), "payload.bin") as uploaded:
        imported_id = uploaded.fileID
        imported_path = Path(imported_id.path) / imported_id.filename

        assert imported_id.filename == "payload.bin"
        assert file_server.findFile(imported_id)
        assert file_server.getFileSize(imported_id) == len(payload)
        assert imported_path.read_bytes() == payload
        assert not (client_path / "payload.bin").exists()

    assert uploaded.closed
    assert not file_server.findFile(imported_id)
    assert not imported_path.exists()

    options = stipy.ImportFileOptions(storage=stipy.ImportStorage.Virtual)
    with device.upload_data(payload, "virtual-payload.bin", options=options) as uploaded:
        virtual_id = uploaded.fileID
        assert virtual_id.filename.startswith("virtual-payload_")
        assert virtual_id.filename.endswith(".bin")
        assert file_server.findFile(virtual_id)
        assert file_server.getFileSize(virtual_id) == len(payload)

    assert uploaded.closed
    assert not file_server.findFile(virtual_id)


def test_stidevicepy_device_upload_data_validates_arguments(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules

    config = stipy.Configuration()
    config.set("Device Name", "Device Upload Data Validation Device")
    config.set("IP Address", "localhost")
    config.set("Module", "67")
    config.set("Target Server", "Device Upload Data Validation Server")
    config.set("PersistenceManager", "root path", str(tmp_path / "persistence"))

    device = stidevicepy.LocalDevice(config)

    with pytest.raises(TypeError, match="bytes-like"):
        device.upload_data("not binary", "payload.bin")
    with pytest.raises(ValueError, match="file name"):
        device.upload_data(b"payload", "")


def test_stidevicepy_persistence_manager_exposes_base_path(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules

    config = stipy.Configuration()
    config.set("Device Name", "Persistence Manager BasePath Device")
    config.set("IP Address", "localhost")
    config.set("Module", "55")
    config.set("Target Server", "Persistence Manager BasePath Server")
    config.set("PersistenceManager", "root path", str(tmp_path))
    config.set("PersistenceManager", "device subdirectory", "device")

    device = stidevicepy.LocalDevice(config)
    persistence = device.getPersistenceManager()

    assert persistence is not None
    assert Path(persistence.getBasePath()) == tmp_path / "device"

    holder = persistence.makeFileHolder(persistence.getBasePath(), "payload.txt")
    assert holder is not None
    assert holder.openFile()
    assert holder.writeText("payload")
    holder.closeFile()

    assert Path(holder.getFilename()) == tmp_path / "device" / "payload.txt"


def test_stidevicepy_persistence_manager_default_base_path_uses_cwd_sti(
    stipy_modules,
    tmp_path,
    monkeypatch,
):
    stipy, stidevicepy = stipy_modules

    monkeypatch.chdir(tmp_path)

    device_name = "Persistence Manager Default BasePath Device"
    config = stipy.Configuration()
    config.set("Device Name", device_name)
    config.set("IP Address", "localhost")
    config.set("Module", "60")
    config.set("Target Server", "Persistence Manager Default BasePath Server")

    device = stidevicepy.LocalDevice(config)
    persistence = device.getPersistenceManager()

    expected_base_path = tmp_path / ".sti" / "localhost" / "60" / device_name
    base_path = Path(persistence.getBasePath())

    assert base_path == expected_base_path
    assert base_path.is_absolute()
    assert base_path.exists()

    holder = persistence.makeFileHolder(persistence.getBasePath(), "payload.txt")
    assert holder is not None
    assert holder.openFile()
    assert holder.writeText("payload")
    holder.closeFile()

    assert Path(holder.getFilename()) == expected_base_path / "payload.txt"
    assert not (tmp_path / "payload.txt").exists()


def test_stidevicepy_persistence_manager_exposes_temporary_path(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules

    config = stipy.Configuration()
    config.set("Device Name", "Persistence Manager Temporary Path Device")
    config.set("IP Address", "localhost")
    config.set("Module", "62")
    config.set("Target Server", "Persistence Manager Temporary Path Server")
    config.set("PersistenceManager", "root path", str(tmp_path))
    config.set("PersistenceManager", "device subdirectory", "device")

    device = stidevicepy.LocalDevice(config)
    persistence = device.getPersistenceManager()

    base_path = Path(persistence.getBasePath())
    temporary_path = Path(persistence.getTemporaryPath())

    assert base_path == tmp_path / "device"
    assert temporary_path == base_path / "transient_cache" / "tmp"
    assert temporary_path.is_absolute()
    assert temporary_path.exists()
    assert temporary_path != base_path

    holder = persistence.makeFileHolder(persistence.getTemporaryPath(), "payload.txt")
    assert holder is not None
    assert holder.openFile()
    assert holder.writeText("payload")
    holder.closeFile()

    assert Path(holder.getFilename()) == temporary_path / "payload.txt"
    assert not (base_path / "payload.txt").exists()


def test_stidevicepy_persistence_manager_relative_root_path_is_absolute(
    stipy_modules,
    tmp_path,
    monkeypatch,
):
    stipy, stidevicepy = stipy_modules

    monkeypatch.chdir(tmp_path)

    config = stipy.Configuration()
    config.set("Device Name", "Persistence Manager Relative BasePath Device")
    config.set("IP Address", "localhost")
    config.set("Module", "61")
    config.set("Target Server", "Persistence Manager Relative BasePath Server")
    config.set("PersistenceManager", "root path", ".relative-sti")
    config.set("PersistenceManager", "device subdirectory", "device")

    device = stidevicepy.LocalDevice(config)
    persistence = device.getPersistenceManager()

    base_path = Path(persistence.getBasePath())
    assert base_path == tmp_path / ".relative-sti" / "device"
    assert base_path.is_absolute()


def test_stidevicepy_file_server_can_transfer_registered_virtual_file(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules

    config = stipy.Configuration()
    config.set("Device Name", "Persistence Manager Virtual File Device")
    config.set("IP Address", "localhost")
    config.set("Module", "56")
    config.set("Target Server", "Persistence Manager Virtual File Server")
    config.set("PersistenceManager", "root path", str(tmp_path))
    config.set("PersistenceManager", "device subdirectory", "device")

    device = stidevicepy.LocalDevice(config)
    persistence = device.getPersistenceManager()
    file_server = persistence.getFileServer()

    source = device.makeVirtualFileHolder("readWrite", "virtual-payload.txt")
    payload = b"virtual payload\nsecond line\n"
    assert source is not None
    assert source.openFile()
    assert source.writeBytes(payload)
    source.closeFile()

    source_id = source.getID()
    assert file_server.addFile(source)

    assert file_server.findFile(source_id)
    assert file_server.getFileSize(source_id) == len(payload)

    backing_destination = stipy.VirtualFileHolder("python-virtual-receiver", source_id)
    destination = persistence.makeVirtualFileHolder(backing_destination)
    assert destination is not None

    assert file_server.transferFile(source_id, destination, stipy.FileTransferType.Binary)
    assert backing_destination.getBytes() == payload


@pytest.mark.integration
@pytest.mark.requires_nameservice
def test_stidevicepy_remote_persistence_manager_exposes_base_path(sti_nameservice_address, stipy_modules):
    stipy, _ = stipy_modules

    server_spec = make_server_spec(name="Persistence Manager Remote BasePath Server", address="localhost", module=57)
    server_id = server_spec.device_id()
    device_spec = DeviceSpec(
        name="Persistence Manager Remote BasePath Device",
        address="localhost",
        module=58,
        target_server_id=server_id.getID(),
        output_channels=[],
        input_channels=[],
        device_subdirectory="remote-base-path-device",
    )

    with InProcessTopology(sti_nameservice_address, server_spec, [device_spec]) as topology:
        wait_for_device_ids(
            topology.hub,
            [server_id, device_spec.device_id()],
            timeout_s=5.0,
            diagnostics=topology.diagnostics,
        )

        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        remote_device = server.getDeviceCollection().get(device_spec.device_id())
        remote_persistence = remote_device.getPersistenceManager()
        local_persistence = topology.devices[0].getPersistenceManager()

        remote_base_path = remote_persistence.getBasePath()
        local_base_path = local_persistence.getBasePath()
        remote_temporary_path = remote_persistence.getTemporaryPath()
        local_temporary_path = local_persistence.getTemporaryPath()

        assert remote_base_path == local_base_path
        assert remote_base_path.endswith("remote-base-path-device")
        assert remote_temporary_path == local_temporary_path
        assert remote_temporary_path.endswith("remote-base-path-device/transient_cache/tmp")

        del remote_persistence
        del local_persistence
        del remote_device
        del server
