import io
from pathlib import Path
from uuid import uuid4

from stipy.stidevicepy.stidevicepy import Device
from stipy.stipybase.stipybase import LocalFileHolder, VirtualFileHolder


def _upload_file(self, filename, *, options=None):
    """Upload a local file into this device's persistence storage.

    The returned ImportedFile owns the target-side file registration and should
    normally be used as a context manager while its FileID is passed to device
    reads or writes.
    """
    source_path = Path(filename).expanduser().resolve(strict=True)
    if not source_path.is_file():
        raise IsADirectoryError(str(source_path))

    persistence = self.getPersistenceManager()
    if persistence is None:
        raise RuntimeError("Device does not provide a PersistenceManager")

    source_holder = persistence.makeFileHolder(
        str(source_path.parent),
        source_path.name,
    )
    source_server = persistence.makeVirtualFileServer()

    if source_holder is None:
        raise RuntimeError(f"Could not create a FileHolder for local file: {source_path}")
    if not source_holder.exists():
        raise FileNotFoundError(source_path)
    if source_server is None:
        raise RuntimeError("Could not create a source FileServer")
    if not source_server.addFile(source_holder):
        raise RuntimeError(f"Could not serve local file: {source_path}")

    if options is None:
        imported = persistence.importFile(source_holder.getID(), source_server)
    else:
        imported = persistence.importFile(source_holder.getID(), source_server, options)

    if imported is None:
        raise RuntimeError(f"Could not upload local file to device: {source_path}")

    return imported


def _upload_data(self, payload, filename, *, options=None):
    """Upload bytes-like data into this device's persistence storage.

    The source payload is held in memory. The returned ImportedFile owns the
    target-side file registration and should normally be used as a context
    manager while its FileID is passed to device reads or writes.
    """
    try:
        data = memoryview(payload).tobytes()
    except TypeError as error:
        raise TypeError(
            "payload must be a bytes-like object supporting the buffer protocol"
        ) from error

    try:
        source_filename = Path(filename).name
    except TypeError as error:
        raise TypeError("filename must be a string or path-like object") from error
    if not source_filename:
        raise ValueError("filename must include a file name")

    persistence = self.getPersistenceManager()
    if persistence is None:
        raise RuntimeError("Device does not provide a PersistenceManager")

    origin = "stipy-upload-data-" + uuid4().hex
    identity_holder = LocalFileHolder(origin, "", source_filename)
    backing_holder = VirtualFileHolder(origin, identity_holder.getID())

    if not backing_holder.openFile():
        raise RuntimeError(f"Could not open in-memory source file: {source_filename}")
    try:
        if not backing_holder.writeBytes(data):
            raise RuntimeError(f"Could not write in-memory source file: {source_filename}")
    finally:
        backing_holder.closeFile()

    source_holder = persistence.makeVirtualFileHolder(backing_holder)
    source_server = persistence.makeVirtualFileServer()

    if source_holder is None:
        raise RuntimeError(
            f"Could not create a FileHolder for in-memory data: {source_filename}"
        )
    if source_server is None:
        raise RuntimeError("Could not create a source FileServer")
    if not source_server.addFile(source_holder):
        raise RuntimeError(f"Could not serve in-memory data: {source_filename}")

    if options is None:
        imported = persistence.importFile(source_holder.getID(), source_server)
    else:
        imported = persistence.importFile(source_holder.getID(), source_server, options)

    if imported is None:
        raise RuntimeError(
            f"Could not upload in-memory data to device: {source_filename}"
        )

    return imported


class VirtualFile(io.StringIO):
    def __init__(self, initial_value='', file_id=None):
        super().__init__(initial_value)
        self._file_id = file_id

    @property
    def file_id(self):
        return self._file_id

    @property
    def file_data(self):
        # Go to the beginning, read all, then reset position
        current_position = self.tell()
        self.seek(0)
        data = self.read()
        self.seek(current_position)
        return data


setattr(Device, "upload_file", _upload_file)
setattr(Device, "upload_data", _upload_data)
