"""STIPy file-backed makeshot behavior."""

import importlib.util
import json
from pathlib import Path
import sys
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


def _module_names():
    suffix = uuid.uuid4().hex
    return "helper_{0}".format(suffix), "main_{0}".format(suffix)


def _write(path, lines):
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _write_var_helper(path, value):
    _write(
        path,
        [
            "from stipy import *",
            "setvar('helper_var', {0})".format(value),
        ],
    )


def _write_importing_main(path, helper_name):
    _write(
        path,
        [
            "from stipy import *",
            "import {0}".format(helper_name),
        ],
    )


def _load_module_from_file(name, path):
    spec = importlib.util.spec_from_file_location(name, str(path))
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def _write_timing_files(tmp_path):
    helper_name, main_name = _module_names()

    helper_file = tmp_path / "{0}.py".format(helper_name)
    main_file = tmp_path / "{0}.py".format(main_name)

    _write(
        helper_file,
        [
            "from stipy import *",
            "settag('helper tag')",
        ],
    )
    _write(
        main_file,
        [
            "from stipy import *",
            "from {0} import *".format(helper_name),
            "settag('main tag')",
        ],
    )
    return helper_file, main_file


def test_file_makeshot_reexecutes_helper_top_level_setvar(stipy_modules, tmp_path):
    stipy, _ = stipy_modules
    helper_name, main_name = _module_names()
    helper_file = tmp_path / "{0}.py".format(helper_name)
    main_file = tmp_path / "{0}.py".format(main_name)
    _write_var_helper(helper_file, 1)
    _write_importing_main(main_file, helper_name)

    first_shot = stipy.makeshot(str(main_file))
    second_shot = stipy.makeshot(str(main_file))

    assert first_shot.rootgroup().var("helper_var") == 1
    assert second_shot.rootgroup().var("helper_var") == 1


def test_file_makeshot_picks_up_changed_helper(stipy_modules, tmp_path):
    stipy, _ = stipy_modules
    helper_name, main_name = _module_names()
    helper_file = tmp_path / "{0}.py".format(helper_name)
    main_file = tmp_path / "{0}.py".format(main_name)
    _write_var_helper(helper_file, 1)
    _write_importing_main(main_file, helper_name)

    first_shot = stipy.makeshot(str(main_file))
    _write_var_helper(helper_file, 2000)
    importlib.invalidate_caches()
    second_shot = stipy.makeshot(str(main_file))

    assert first_shot.rootgroup().var("helper_var") == 1
    assert second_shot.rootgroup().var("helper_var") == 2000


def test_file_makeshot_ignores_stale_preimported_helper(stipy_modules, tmp_path):
    stipy, _ = stipy_modules
    helper_name, main_name = _module_names()
    helper_file = tmp_path / "{0}.py".format(helper_name)
    main_file = tmp_path / "{0}.py".format(main_name)
    _write(
        helper_file,
        [
            "HELPER_VALUE = 1",
        ],
    )
    _load_module_from_file(helper_name, helper_file)
    _write_var_helper(helper_file, 2000)
    _write_importing_main(main_file, helper_name)
    importlib.invalidate_caches()

    shot = stipy.makeshot(str(main_file))

    assert shot.rootgroup().var("helper_var") == 2000


def test_file_makeshot_cleans_timing_modules_from_sys_modules(stipy_modules, tmp_path):
    stipy, _ = stipy_modules
    helper_name, main_name = _module_names()
    helper_file = tmp_path / "{0}.py".format(helper_name)
    main_file = tmp_path / "{0}.py".format(main_name)
    _write_var_helper(helper_file, 1)
    _write_importing_main(main_file, helper_name)

    stipy.makeshot(str(main_file))

    assert helper_name not in sys.modules
    assert main_name not in sys.modules


def test_file_makeshot_keeps_stable_modules_in_sys_modules(stipy_modules, tmp_path):
    stipy, _ = stipy_modules
    helper_name, main_name = _module_names()
    helper_file = tmp_path / "{0}.py".format(helper_name)
    main_file = tmp_path / "{0}.py".format(main_name)
    _write_var_helper(helper_file, 1)
    _write_importing_main(main_file, helper_name)
    stipy_module = sys.modules["stipy"]
    json_module = sys.modules["json"]

    stipy.makeshot(str(main_file))

    assert sys.modules["stipy"] is stipy_module
    assert sys.modules["json"] is json_module
    numpy_module = sys.modules.get("numpy")
    if numpy_module is not None:
        assert sys.modules["numpy"] is numpy_module


def test_file_makeshot_reexecutes_helpers_from_extra_import_roots(stipy_modules, tmp_path):
    stipy, _ = stipy_modules
    timing_dir = tmp_path / "timing"
    shared_dir = tmp_path / "shared_timing"
    timing_dir.mkdir()
    shared_dir.mkdir()
    helper_name, main_name = _module_names()
    helper_file = shared_dir / "{0}.py".format(helper_name)
    main_file = timing_dir / "{0}.py".format(main_name)
    _write_var_helper(helper_file, 1)
    _write_importing_main(main_file, helper_name)

    first_shot = stipy.makeshot(str(main_file), import_roots=[shared_dir])
    _write_var_helper(helper_file, 2000)
    importlib.invalidate_caches()
    second_shot = stipy.makeshot(str(main_file), import_roots=[shared_dir])

    assert first_shot.rootgroup().var("helper_var") == 1
    assert second_shot.rootgroup().var("helper_var") == 2000
    assert helper_name not in sys.modules


def test_file_makeshot_main_globals_do_not_leak(stipy_modules, tmp_path):
    stipy, _ = stipy_modules
    _, main_a_name = _module_names()
    _, main_b_name = _module_names()
    main_a_file = tmp_path / "{0}.py".format(main_a_name)
    main_b_file = tmp_path / "{0}.py".format(main_b_name)
    leaked_name = "stipy_makeshot_test_global_{0}".format(uuid.uuid4().hex)
    _write(
        main_a_file,
        [
            "{0} = 1".format(leaked_name),
        ],
    )
    _write(
        main_b_file,
        [
            "{0}".format(leaked_name),
        ],
    )

    stipy.makeshot(str(main_a_file))

    assert leaked_name not in globals()
    with pytest.raises(NameError):
        stipy.makeshot(str(main_b_file))


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


@pytest.mark.integration
@pytest.mark.requires_nameservice
def test_server_file_makeshot_reexecutes_helper_top_level_setvar(
    sti_nameservice_address, stipy_modules, tmp_path
):
    helper_name, main_name = _module_names()
    helper_file = tmp_path / "{0}.py".format(helper_name)
    main_file = tmp_path / "{0}.py".format(main_name)
    _write_var_helper(helper_file, 1)
    _write_importing_main(main_file, helper_name)
    server_spec = make_server_spec(name="Repeated File Makeshot Server", address="localhost", module=71)
    server_id = server_spec.device_id()

    with InProcessTopology(sti_nameservice_address, server_spec, []) as topology:
        wait_for_device_ids(topology.hub, [server_id], timeout_s=5.0, diagnostics=topology.diagnostics)

        server = topology.connect_stipy()
        assert server is not None, topology.summary()

        first_shot = server.makeshot(str(main_file))
        second_shot = server.makeshot(str(main_file))

        assert first_shot.rootgroup().var("helper_var") == 1
        assert second_shot.rootgroup().var("helper_var") == 1
