"""stidevicepy persistence manager API coverage."""


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
