"""DeviceCollection Python wrapper behavior."""


def test_device_collection_get_missing_id_returns_none(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules

    config = stipy.Configuration()
    config.set("Device Name", "Collection Wrapper Device")
    config.set("IP Address", "localhost")
    config.set("Module", "90")
    config.set("Target Server", "root")
    config.set("PersistenceManager", "root path", str(tmp_path))
    config.set("PersistenceManager", "device subdirectory", "device")
    config.set("EngineManager", "Engine Count", "0")

    device = stidevicepy.LocalDevice(config)
    collection = device.getDeviceCollection()

    assert collection.get(stipy.DeviceID("Missing Device", "localhost", 91, "root")) is None
