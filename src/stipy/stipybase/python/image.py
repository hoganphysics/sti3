from pathlib import Path
import io
import tempfile
import uuid

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


def _metadata_value(image, key):
    if not hasattr(image, "metadata"):
        return None

    try:
        return image.metadata(key)
    except Exception:
        return None


def _is_raw_encoding(encoding):
    encoding = _normalized_format(encoding)
    return encoding in {"RAW", "PIXELS", "BYTES"}


def _positive_dimension(value):
    try:
        value = int(value)
    except (TypeError, ValueError):
        return None

    return value if value > 0 else None


def _infer_raw_mode(payload_size, width, height):
    pixel_count = width * height
    if pixel_count <= 0:
        return None

    bytes_per_pixel = {
        1: "L",
        3: "RGB",
        4: "RGBA",
    }
    return bytes_per_pixel.get(payload_size // pixel_count) if payload_size % pixel_count == 0 else None


def _raw_mode_payload_size(mode, width, height):
    mode = str(mode)
    if mode == "1":
        return ((width + 7) // 8) * height

    bytes_per_pixel = {
        "L": 1,
        "P": 1,
        "LA": 2,
        "I;16": 2,
        "I;16L": 2,
        "I;16B": 2,
        "RGB": 3,
        "RGBA": 4,
        "CMYK": 4,
        "I": 4,
        "F": 4,
    }.get(mode)

    return None if bytes_per_pixel is None else width * height * bytes_per_pixel


def _raw_payload_to_pil(payload, width, height, mode=None):
    from PIL import Image as PILImage

    width = _positive_dimension(width)
    height = _positive_dimension(height)
    if width is None or height is None:
        raise ValueError(
            "Raw STI_Image BinaryData needs positive width and height metadata"
        )

    if mode is not None:
        mode = str(mode)
        expected_size = _raw_mode_payload_size(mode, width, height)
        if expected_size is not None and expected_size != len(payload):
            raise ValueError(
                "Raw STI_Image payload size does not match mode, width, and height"
            )
    else:
        mode = _infer_raw_mode(len(payload), width, height)
        if mode is None:
            raise ValueError(
                "Raw STI_Image BinaryData needs mode metadata when payload size "
                "does not imply L, RGB, or RGBA pixels"
            )

    image = PILImage.frombytes(mode, (width, height), payload)
    image.load()
    return image


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
            "mode": image.mode,
        },
    )


def _looks_like_persistence_manager(value):
    return (
        hasattr(value, "getFileServer")
        and hasattr(value, "makeVirtualFileHolder")
        and hasattr(value, "makeFileHolder")
    )


def _resolve_persistence(context=None, persistence=None):
    if persistence is not None:
        return persistence

    if context is None:
        return None

    if _looks_like_persistence_manager(context):
        return context

    if hasattr(context, "getPersistenceManager"):
        return context.getPersistenceManager()

    return None


def _resolve_transfer_context(
    context=None,
    *,
    persistence=None,
    file_server=None,
    destination_factory=None,
):
    resolved_persistence = _resolve_persistence(context, persistence)
    resolved_file_server = file_server
    resolved_destination_factory = destination_factory

    if resolved_file_server is None and resolved_persistence is not None:
        resolved_file_server = resolved_persistence.getFileServer()

    if resolved_destination_factory is None:
        resolved_destination_factory = resolved_persistence

    if resolved_file_server is None:
        raise ValueError(
            "STI_Image transfer needs a FileServer. Pass a Device/STIPyServer, "
            "PersistenceManager, or file_server=..."
        )

    if resolved_destination_factory is None:
        raise ValueError(
            "STI_Image transfer needs a FileHolder factory. Pass a "
            "Device/STIPyServer, PersistenceManager, or destination_factory=..."
        )

    return resolved_file_server, resolved_destination_factory


def _image_encoding(self):
    return _metadata_value(self, "encoding") or _metadata_value(self, "format")


def _image_suffix(self):
    image_format = _image_encoding(self)
    return "." + str(image_format).lower() if image_format else ""


def _image_data_payload(self):
    data = self.getData() if self.hasData() else None
    if data is None:
        return None

    data.pull()
    payload = data.getBytes()
    if payload is None:
        raise ValueError("STI_Image BinaryData has no readable bytes")

    return payload


def _decode_payload_to_pil(self, payload, pil_image_module):
    encoding = _image_encoding(self)
    if _is_raw_encoding(encoding):
        return _raw_payload_to_pil(
            payload,
            self.getWidth(),
            self.getHeight(),
            _metadata_value(self, "mode"),
        )

    try:
        image = pil_image_module.open(io.BytesIO(payload))
        image.load()
        return image
    except Exception:
        if encoding:
            raise
        return _raw_payload_to_pil(
            payload,
            self.getWidth(),
            self.getHeight(),
            _metadata_value(self, "mode"),
        )


def _decode_file_to_pil(self, path, pil_image_module):
    image_format = _image_encoding(self)
    if _is_raw_encoding(image_format):
        return _raw_payload_to_pil(
            Path(path).read_bytes(),
            self.getWidth(),
            self.getHeight(),
            _metadata_value(self, "mode"),
        )

    try:
        image = pil_image_module.open(path)
        image.load()
        return image
    except Exception:
        if image_format:
            raise
        return _raw_payload_to_pil(
            Path(path).read_bytes(),
            self.getWidth(),
            self.getHeight(),
            _metadata_value(self, "mode"),
        )


def _local_image_payload(self):
    payload = _image_data_payload(self)
    if payload is not None:
        return payload

    with tempfile.NamedTemporaryFile(suffix=_image_suffix(self)) as preview_file:
        if not self.save(preview_file.name):
            return None

        return Path(preview_file.name).read_bytes()


def _transfer_image_to_virtual_bytes(
    self,
    context=None,
    *,
    persistence=None,
    file_server=None,
    destination_factory=None,
):
    from ..stipybase import FileTransferType, VirtualFileHolder

    source_file_server, factory = _resolve_transfer_context(
        context,
        persistence=persistence,
        file_server=file_server,
        destination_factory=destination_factory,
    )

    source_file_id = self.getFileID()
    backing_holder = VirtualFileHolder(
        "stipy-image-download-" + uuid.uuid4().hex,
        source_file_id,
    )
    destination_holder = factory.makeVirtualFileHolder(backing_holder)

    if destination_holder is None:
        raise ValueError("STI_Image transfer could not create a virtual destination")

    success = source_file_server.transferFile(
        source_file_id,
        destination_holder,
        FileTransferType.Binary,
    )
    if not success:
        raise RuntimeError("Failed to transfer STI_Image file " + source_file_id.filename)

    payload = backing_holder.getBytes()
    if payload is None:
        raise RuntimeError("STI_Image transfer completed without readable bytes")

    return payload


def _transfer_image_to_file(
    self,
    path,
    context=None,
    *,
    persistence=None,
    file_server=None,
    destination_factory=None,
):
    from ..stipybase import FileTransferType

    target_path = Path(path)
    source_file_server, factory = _resolve_transfer_context(
        context,
        persistence=persistence,
        file_server=file_server,
        destination_factory=destination_factory,
    )

    destination_holder = factory.makeFileHolder(
        str(target_path.parent),
        target_path.name,
    )
    if destination_holder is None:
        raise ValueError("STI_Image transfer could not create a file destination")

    source_file_id = self.getFileID()
    success = source_file_server.transferFile(
        source_file_id,
        destination_holder,
        FileTransferType.Binary,
    )
    if not success:
        raise RuntimeError("Failed to transfer STI_Image file " + source_file_id.filename)

    return target_path


def _image_to_bytes(
    self,
    context=None,
    *,
    persistence=None,
    file_server=None,
    destination_factory=None,
):
    """Return image payload bytes, transferring FileID-backed images when needed."""

    payload = _local_image_payload(self)
    if payload is not None:
        return payload

    return _transfer_image_to_virtual_bytes(
        self,
        context,
        persistence=persistence,
        file_server=file_server,
        destination_factory=destination_factory,
    )


def _image_to_file(
    self,
    path,
    context=None,
    *,
    persistence=None,
    file_server=None,
    destination_factory=None,
):
    """Save or transfer this image to path and return the written Path."""

    target_path = Path(path)

    if self.save(str(target_path)):
        return target_path

    return _transfer_image_to_file(
        self,
        target_path,
        context,
        persistence=persistence,
        file_server=file_server,
        destination_factory=destination_factory,
    )


def _image_to_pil(
    self,
    context=None,
    *,
    persistence=None,
    file_server=None,
    destination_factory=None,
    storage="virtual",
    path=None,
):
    """Convert to PIL.Image, using a Python-owned virtual transfer by default."""

    try:
        from PIL import Image as PILImage
    except ImportError as exc:
        raise ImportError(
            "Pillow is optional. Install Pillow to convert STI_Image to PIL.Image."
        ) from exc

    storage = str(storage).lower()
    if storage not in {"virtual", "memory", "file", "disk"}:
        raise ValueError("storage must be 'virtual' or 'file'")

    if storage in {"virtual", "memory"} and path is None:
        payload = _image_to_bytes(
            self,
            context,
            persistence=persistence,
            file_server=file_server,
            destination_factory=destination_factory,
        )
        return _decode_payload_to_pil(self, payload, PILImage)

    if path is not None:
        image_path = _image_to_file(
            self,
            path,
            context,
            persistence=persistence,
            file_server=file_server,
            destination_factory=destination_factory,
        )
        return _decode_file_to_pil(self, image_path, PILImage)

    with tempfile.TemporaryDirectory() as preview_dir:
        preview_file = Path(preview_dir) / (
            self.getFileID().filename or ("sti-image-preview" + _image_suffix(self))
        )
        _image_to_file(
            self,
            preview_file,
            context,
            persistence=persistence,
            file_server=file_server,
            destination_factory=destination_factory,
        )
        return _decode_file_to_pil(self, preview_file, PILImage)


def _install_image_helpers():
    setattr(STI_Image, "from_file", classmethod(_image_from_file))
    setattr(STI_Image, "from_path", classmethod(_image_from_file))
    setattr(STI_Image, "from_pil", classmethod(_image_from_pil))
    setattr(STI_Image, "to_bytes", _image_to_bytes)
    setattr(STI_Image, "to_file", _image_to_file)
    setattr(STI_Image, "to_pil", _image_to_pil)


_install_image_helpers()
