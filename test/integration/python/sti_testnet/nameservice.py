"""omniORB name-service lifecycle helpers for STI integration tests."""

import os
import shutil
import socket
import subprocess
import tempfile
import time


class NameServiceError(RuntimeError):
    pass


class SpawnedNameService(object):
    def __init__(self, host="127.0.0.1", port=None, timeout_s=5.0, datadir=None):
        self.host = host
        self.port = int(port) if port is not None else None
        self.timeout_s = float(timeout_s)
        self.datadir = datadir
        self._owns_datadir = datadir is None
        self.process = None
        self.errlog = None

    @property
    def address(self):
        if self.port is None:
            return None
        return "{0}:{1}".format(self.host, self.port)

    def start(self):
        if self.process is not None:
            return self
        if self.port is None:
            self.port = _find_unused_non_default_port(self.host)
        if self.datadir is None:
            self.datadir = tempfile.mkdtemp(prefix="sti3-omniorb-nameservice-")
        self.errlog = os.path.join(self.datadir, "omniNames.err")

        command = [
            "omniNames",
            "-start",
            str(self.port),
            "-always",
            "-datadir",
            self.datadir,
            "-errlog",
            self.errlog,
            "-ORBendPointPublish",
            "giop:tcp:{0}:".format(self.host),
        ]
        self.process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            close_fds=True,
        )
        try:
            self._wait_until_ready()
        except Exception:
            self.shutdown()
            raise
        return self

    def shutdown(self):
        process = self.process
        self.process = None
        if process is not None and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=2.0)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=2.0)
        if self._owns_datadir and self.datadir is not None:
            shutil.rmtree(self.datadir, ignore_errors=True)
            self.datadir = None

    def keep_alive_note(self):
        return "Spawned omniNames left running at {0}; datadir: {1}".format(self.address, self.datadir)

    def _wait_until_ready(self):
        deadline = time.time() + self.timeout_s
        last_error = None
        while time.time() < deadline:
            if self.process.poll() is not None:
                raise NameServiceError(self._failure_message("omniNames exited before becoming ready"))
            try:
                connection = socket.create_connection((self.host, self.port), timeout=0.25)
                connection.close()
                return
            except OSError as exc:
                last_error = exc
                time.sleep(0.05)
        raise NameServiceError(self._failure_message("timed out waiting for omniNames: {0}".format(last_error)))

    def _failure_message(self, reason):
        parts = [reason, "address: {0}".format(self.address), "datadir: {0}".format(self.datadir)]
        stdout, stderr = self._read_process_output()
        errlog = self._read_file(self.errlog)
        if stdout:
            parts.append("stdout:\n{0}".format(stdout))
        if stderr:
            parts.append("stderr:\n{0}".format(stderr))
        if errlog:
            parts.append("errlog:\n{0}".format(errlog))
        return "\n".join(parts)

    def _read_process_output(self):
        if self.process is None or self.process.poll() is None:
            return "", ""
        stdout, stderr = self.process.communicate()
        return _decode_output(stdout), _decode_output(stderr)

    def _read_file(self, filename):
        if not filename or not os.path.exists(filename):
            return ""
        try:
            with open(filename, "r") as handle:
                return handle.read()
        except OSError:
            return ""


def _find_unused_non_default_port(host):
    for _ in range(20):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            sock.bind((host, 0))
            _, port = sock.getsockname()
        finally:
            sock.close()
        if int(port) != 2809:
            return int(port)
    raise NameServiceError("could not allocate a non-default local port")


def _decode_output(output):
    if not output:
        return ""
    if isinstance(output, str):
        return output
    return output.decode("utf-8", "replace")
