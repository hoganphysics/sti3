from pathlib import Path
import io
import tempfile

from ..stipybase import Image as _Image


STI_Image = _Image


def _pil_image_type():
    try:
        from PIL import Image as PILImage
    except ImportError as exc:
        raise ImportError(
            "Pillow is optional. Install Pillow to construct an STI_Image "
            "from a PIL image object."
        ) from exc

    return PILImage.Image


def _probe_file_with_pillow(path):
    try:
        from PIL import Image as PILImage
    except ImportError:
        return None

    try:
        with PILImage.open(path) as image:
            return image.width, image.height, image.format
    except Exception:
        return None


def _set_image_metadata(image, entries):
    if not hasattr(image, "setMetaData"):
        return image

    for key, value in entries.items():
        if value is not None:
            image.setMetaData(key, value)

    return image


def _normalized_format(format_name):
    if format_name is None:
        return None

    format_name = str(format_name)
    if not format_name:
        return None

    return format_name.upper()


def _image_from_file(
    cls,
    path,
    *,
    width=None,
    height=None,
    format=None,
    infer_dimensions=True,
    storage="binary",
):
    if storage != "binary":
        raise ValueError("STI_Image.from_file currently supports storage='binary' only")

    image_path = Path(path)
    payload = image_path.read_bytes()
    image_format = _normalized_format(format)

    if infer_dimensions and (width is None or height is None or image_format is None):
        probed = _probe_file_with_pillow(image_path)
        if probed is not None:
            probed_width, probed_height, probed_format = probed
            if width is None:
                width = probed_width
            if height is None:
                height = probed_height
            if image_format is None:
                image_format = _normalized_format(probed_format)

    if image_format is None and image_path.suffix:
        image_format = image_path.suffix.lstrip(".").upper()

    image = cls(payload, width=width or 0, height=height or 0)
    return _set_image_metadata(
        image,
        {
            "source": "file",
            "source_filename": image_path.name,
            "format": image_format,
            "storage": "BinaryData",
            "encoding": image_format,
        },
    )


def _image_from_pil(cls, image, *, format=None, **save_kwargs):
    pil_type = _pil_image_type()
    if not isinstance(image, pil_type):
        raise TypeError("Expected a PIL.Image.Image object")

    image_format = _normalized_format(format or image.format or "PNG")
    buffer = io.BytesIO()
    image.save(buffer, format=image_format, **save_kwargs)

    sti_image = cls(buffer.getvalue(), width=image.width, height=image.height)
    return _set_image_metadata(
        sti_image,
        {
            "source": "PIL.Image",
            "format": image_format,
            "storage": "BinaryData",
            "encoding": image_format,
        },
    )


def _image_to_pil(self):
    try:
        from PIL import Image as PILImage
    except ImportError as exc:
        raise ImportError(
            "Pillow is optional. Install Pillow to convert STI_Image to PIL.Image."
        ) from exc

    data = self.getData() if self.hasData() else None
    if data is not None:
        data.pull()
        payload = data.getBytes()
        if payload is None:
            raise ValueError("STI_Image BinaryData has no readable bytes")

        image = PILImage.open(io.BytesIO(payload))
        image.load()
        return image

    suffix = ""
    if hasattr(self, "metadata"):
        try:
            image_format = self.metadata("format")
        except Exception:
            image_format = None
        if image_format:
            suffix = "." + str(image_format).lower()

    with tempfile.NamedTemporaryFile(suffix=suffix) as preview_file:
        if not self.save(preview_file.name):
            raise ValueError(
                "STI_Image has neither readable BinaryData nor a saveable file payload"
            )

        image = PILImage.open(preview_file.name)
        image.load()
        return image


def _install_image_helpers():
    setattr(STI_Image, "from_file", classmethod(_image_from_file))
    setattr(STI_Image, "from_path", classmethod(_image_from_file))
    setattr(STI_Image, "from_pil", classmethod(_image_from_pil))
    setattr(STI_Image, "to_pil", _image_to_pil)


_install_image_helpers()
