import base64

import pytest


ONE_PIXEL_PNG = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+/p9sAAAAASUVORK5CYII="
)


def local_persistence(stipy, stidevicepy, tmp_path):
    config = stipy.Configuration()
    config.set("Device Name", "STI Image Helper Device")
    config.set("IP Address", "localhost")
    config.set("Module", "63")
    config.set("Target Server", "STI Image Helper Server")
    config.set("PersistenceManager", "root path", str(tmp_path))
    config.set("PersistenceManager", "device subdirectory", "device")

    device = stidevicepy.LocalDevice(config)
    return device, device.getPersistenceManager()


def registered_image_file(stipy, persistence, payload, filename):
    holder = persistence.makeFileHolder(persistence.getTemporaryPath(), filename)
    assert holder is not None
    assert holder.openFile()
    try:
        assert holder.writeBytes(payload)
    finally:
        holder.closeFile()

    assert persistence.getFileServer().addFile(holder)

    image = stipy.STI_Image(holder.getID(), width=1, height=1)
    image.setMetaData("format", "PNG")
    image.setMetaData("encoding", "PNG")
    return image


def test_stipy_star_import_exports_sti_image_alias_without_image(stipy_modules):
    stipy, _ = stipy_modules

    assert stipy.STI_Image is stipy.Image
    assert "STI_Image" in stipy.__all__
    assert "Image" not in stipy.__all__

    namespace = {}
    exec("from stipy import *", namespace)
    assert namespace["STI_Image"] is stipy.Image
    assert "Image" not in namespace


def test_sti_image_from_file_preserves_file_bytes_without_pillow(stipy_modules, tmp_path):
    stipy, _ = stipy_modules

    image_path = tmp_path / "example.raw"
    payload = bytes(range(12))
    image_path.write_bytes(payload)

    image = stipy.STI_Image.from_file(
        image_path,
        width=4,
        height=3,
        infer_dimensions=False,
    )

    assert isinstance(image, stipy.Image)
    assert image.getWidth() == 4
    assert image.getHeight() == 3
    assert image.metadata("source") == "file"
    assert image.metadata("source_filename") == "example.raw"
    assert image.metadata("storage") == "BinaryData"
    assert image.getData().getBytes() == payload

    value = stipy.MixedValue(image)
    assert value.getType() == stipy.MixedValueType.Image
    assert isinstance(value.getImage(), stipy.Image)


def test_sti_image_from_file_holder_keeps_filename_in_file_id(stipy_modules, tmp_path):
    stipy, _ = stipy_modules

    holder = stipy.LocalFileHolder("origin", str(tmp_path), "payload.bin")
    image = stipy.STI_Image(holder, width=1, height=1)

    assert image.getFileID().origin == "origin"
    assert image.getFileID().path == str(tmp_path)
    assert image.getFileID().filename == "payload.bin"


def test_sti_image_from_pil_when_pillow_is_available(stipy_modules):
    pil_image_module = pytest.importorskip("PIL.Image")
    stipy, _ = stipy_modules

    pil_image = pil_image_module.new("RGB", (2, 1), color=(1, 2, 3))

    image = stipy.STI_Image.from_pil(pil_image, format="PNG")
    assert image.getWidth() == 2
    assert image.getHeight() == 1
    assert image.metadata("source") == "PIL.Image"
    assert image.metadata("format") == "PNG"
    assert image.getData().getBytes().startswith(b"\x89PNG")

    converted = image.to_pil()
    assert converted.size == (2, 1)
    assert converted.getpixel((0, 0)) == (1, 2, 3)

    constructed = stipy.STI_Image(pil_image)
    assert constructed.getWidth() == 2
    assert constructed.getHeight() == 1
    assert constructed.metadata("source") == "PIL.Image"
    assert constructed.getData().getBytes().startswith(b"\x89PNG")


def test_sti_image_to_pil_infers_raw_grayscale_binary_payload(stipy_modules):
    pytest.importorskip("PIL.Image")
    stipy, _ = stipy_modules

    image = stipy.STI_Image(bytes([0, 127, 255, 64]), width=2, height=2)

    converted = image.to_pil()

    assert converted.mode == "L"
    assert converted.size == (2, 2)
    assert converted.getpixel((0, 0)) == 0
    assert converted.getpixel((1, 0)) == 127
    assert converted.getpixel((0, 1)) == 255
    assert converted.getpixel((1, 1)) == 64


def test_sti_image_to_pil_uses_raw_mode_metadata(stipy_modules):
    pytest.importorskip("PIL.Image")
    stipy, _ = stipy_modules

    image = stipy.STI_Image(
        bytes([1, 2, 3, 4, 5, 6]),
        width=2,
        height=1,
    )
    image.setMetaData("encoding", "raw")
    image.setMetaData("mode", "RGB")

    converted = image.to_pil()

    assert converted.mode == "RGB"
    assert converted.size == (2, 1)
    assert converted.getpixel((0, 0)) == (1, 2, 3)
    assert converted.getpixel((1, 0)) == (4, 5, 6)


def test_sti_image_to_bytes_transfers_file_id_to_virtual_holder(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules
    device, persistence = local_persistence(stipy, stidevicepy, tmp_path)
    image = registered_image_file(stipy, persistence, ONE_PIXEL_PNG, "context-image.png")

    assert not image.hasData()
    assert not image.hasFile()

    assert image.to_bytes(persistence) == ONE_PIXEL_PNG
    assert image.to_bytes(device) == ONE_PIXEL_PNG


def test_sti_image_to_file_transfers_file_id_to_local_file_holder(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules
    _, persistence = local_persistence(stipy, stidevicepy, tmp_path)
    image = registered_image_file(stipy, persistence, ONE_PIXEL_PNG, "file-image.png")

    output_path = tmp_path / "downloaded.png"

    returned_path = image.to_file(output_path, persistence)

    assert returned_path == output_path
    assert output_path.read_bytes() == ONE_PIXEL_PNG


def test_sti_image_to_pil_transfers_file_id_when_context_is_provided(stipy_modules, tmp_path):
    pytest.importorskip("PIL.Image")
    stipy, stidevicepy = stipy_modules
    device, persistence = local_persistence(stipy, stidevicepy, tmp_path)
    image = registered_image_file(stipy, persistence, ONE_PIXEL_PNG, "pil-image.png")

    converted = image.to_pil(device)
    converted_from_file = image.to_pil(device, storage="file")

    assert converted.size == (1, 1)
    assert converted.mode
    assert converted_from_file.size == (1, 1)


def test_local_device_make_image_result_defaults_to_binarydata(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules
    device, _ = local_persistence(stipy, stidevicepy, tmp_path)

    image = device.makeImageResult(
        ONE_PIXEL_PNG,
        "helper-image.png",
        width=1,
        height=1,
        encoding="PNG",
    )

    assert isinstance(image, stipy.Image)
    assert image.hasData()
    assert image.getFileID().filename == "helper-image.png"
    assert image.metadata("storage") == "BinaryData"
    assert image.to_bytes() == ONE_PIXEL_PNG


def test_local_device_make_virtual_file_result_keeps_only_last_read(stipy_modules, tmp_path):
    stipy, stidevicepy = stipy_modules

    class ReadDevice(stidevicepy.LocalDevice):
        def __init__(self, config):
            stidevicepy.LocalDevice.__init__(self, config)
            self.read_count = 0
            self.addInputChannel(0, stipy.MixedValueType.File, "virtual file")

        def readChannel(self, channel, value):
            self.read_count += 1
            payload = "payload {0}".format(self.read_count).encode("ascii")
            return self.makeFileResult(
                payload,
                "read-{0}.txt".format(self.read_count),
                path="read",
            )

    config = stipy.Configuration()
    config.set("Device Name", "Read Helper Device")
    config.set("IP Address", "localhost")
    config.set("Module", "64")
    config.set("Target Server", "STI Image Helper Server")
    config.set("PersistenceManager", "root path", str(tmp_path))
    config.set("PersistenceManager", "device subdirectory", "device")
    device = ReadDevice(config)
    file_server = device.getPersistenceManager().getFileServer()

    first_file_id = device.read(0)
    assert file_server.findFile(first_file_id)

    second_file_id = device.read(0)
    assert not file_server.findFile(first_file_id)
    assert file_server.findFile(second_file_id)
