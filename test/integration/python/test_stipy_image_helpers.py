import pytest


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
