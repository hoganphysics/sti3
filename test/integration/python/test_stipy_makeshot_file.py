"""STIPy file-backed makeshot behavior."""

from pathlib import Path
import uuid

import pytest

from sti_testnet.devices import make_server_spec
from sti_testnet.topology import InProcessTopology
from sti_testnet.waits import wait_for_device_ids


def _timing_file_paths(shot):
    return {
        str(Path(file_id.path) / file_id.filename)
        for file_id in shot.rootgroup().getStackTraceData().getTimingFiles()
    }


def _write_timing_files(tmp_path):
    suffix = uuid.uuid4().hex
    helper_name = "helper_{0}".format(suffix)
    main_name = "main_{0}".format(suffix)

    helper_file = tmp_path / "{0}.py".format(helper_name)
    main_file = tmp_path / "{0}.py".format(main_name)

    helper_file.write_text(
        "\n".join(
            [
                "from stipy import *",
                "settag('helper tag')",
            ]
        ),
        encoding="utf-8",
    )
    main_file.write_text(
        "\n".join(
            [
                "from stipy import *",
                "from {0} import *".format(helper_name),
                "settag('main tag')",
            ]
        ),
        encoding="utf-8",
    )
    return helper_file, main_file


def test_file_makeshot_uses_submitted_main_file_for_shot_config(stipy_modules, tmp_path):
    stipy, _ = stipy_modules
    helper_file, main_file = _write_timing_files(tmp_path)

    shot = stipy.makeshot(str(main_file))

    assert shot.shotconfig().file == str(main_file.resolve())
    assert str(helper_file.resolve()) in _timing_file_paths(shot)
    assert str(main_file.resolve()) in _timing_file_paths(shot)


@pytest.mark.integration
@pytest.mark.requires_nameservice
def test_server_file_makeshot_uses_submitted_main_file_for_shot_config(
    sti_nameservice_address, stipy_modules, tmp_path
):
    helper_file, main_file = _write_timing_files(tmp_path)
    server_spec = make_server_spec(name="ShotConfig File Server", address="localhost", module=70)
    server_id = server_spec.device_id()

    with InProcessTopology(sti_nameservice_address, server_spec, []) as topology:
        wait_for_device_ids(topology.hub, [server_id], timeout_s=5.0, diagnostics=topology.diagnostics)

        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        shot = server.makeshot(str(main_file))

        assert shot.shotconfig().file == str(main_file.resolve())
        assert str(helper_file.resolve()) in _timing_file_paths(shot)
        assert str(main_file.resolve()) in _timing_file_paths(shot)
