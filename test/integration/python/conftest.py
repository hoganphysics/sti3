"""Pytest configuration for STI network integration tests."""

import pytest


def _resolve_nameservice_mode(config):
    mode = config.getoption("--sti-nameservice-mode")
    nameservice = config.getoption("--sti-nameservice")
    observe = bool(config.getoption("--observe"))

    if mode != "auto":
        return mode
    if nameservice:
        return "external"
    if observe:
        return "external"
    return "spawn"


@pytest.fixture(autouse=True)
def cleanup_integration_persistence_roots():
    yield
    try:
        from sti_testnet.topology import cleanup_registered_persistence_roots
        cleanup_registered_persistence_roots()
    except Exception:
        pass


def pytest_addoption(parser):
    group = parser.getgroup("sti integration")
    group.addoption(
        "--sti-nameservice",
        action="store",
        default=None,
        help="Attach tests to an existing omniORB name service, such as 127.0.0.1:2809.",
    )
    group.addoption(
        "--sti-nameservice-mode",
        action="store",
        default="auto",
        choices=("auto", "external", "spawn"),
        help="Select name-service handling: auto, external, or spawn.",
    )
    group.addoption(
        "--observe",
        action="store_true",
        default=False,
        help="Enable tests marked observe for frontend inspection.",
    )
    group.addoption(
        "--observe-timeout",
        action="store",
        type=float,
        default=300.0,
        help="Maximum seconds to keep an observe-mode topology alive.",
    )
    group.addoption(
        "--keep-network-alive",
        action="store_true",
        default=False,
        help="Leave spawned test network processes alive after setup for manual inspection.",
    )


def pytest_collection_modifyitems(config, items):
    markexpr = config.option.markexpr or ""
    skip_slow = pytest.mark.skip(reason="slow integration tests require an explicit slow marker selection")
    skip_stress = pytest.mark.skip(reason="stress integration tests require -m stress")

    if config.getoption("--observe"):
        skip_observe = None
    else:
        skip_observe = pytest.mark.skip(reason="observe-mode tests require --observe")

    for item in items:
        if skip_observe is not None and "observe" in item.keywords:
            item.add_marker(skip_observe)
        if "slow" in item.keywords and "slow" not in markexpr:
            item.add_marker(skip_slow)
        if "stress" in item.keywords and "stress" not in markexpr:
            item.add_marker(skip_stress)


@pytest.fixture
def sti_nameservice(pytestconfig):
    return pytestconfig.getoption("--sti-nameservice")


@pytest.fixture
def sti_nameservice_mode(pytestconfig):
    return pytestconfig.getoption("--sti-nameservice-mode")


@pytest.fixture
def resolved_sti_nameservice_mode(pytestconfig):
    return _resolve_nameservice_mode(pytestconfig)


@pytest.fixture(scope="session")
def spawned_sti_nameservice(pytestconfig):
    if _resolve_nameservice_mode(pytestconfig) != "spawn":
        yield None
        return

    from sti_testnet.nameservice import SpawnedNameService

    service = SpawnedNameService()
    service.start()
    try:
        yield service.address
    finally:
        if bool(pytestconfig.getoption("--keep-network-alive")):
            print("\n" + service.keep_alive_note())
        else:
            service.shutdown()


@pytest.fixture
def external_sti_nameservice(sti_nameservice, resolved_sti_nameservice_mode):
    if resolved_sti_nameservice_mode != "external":
        pytest.skip("test requires an external omniORB name service")
    if not sti_nameservice:
        pytest.skip("pass --sti-nameservice host:port to use an external omniORB name service")
    return sti_nameservice


@pytest.fixture
def sti_nameservice_address(sti_nameservice, resolved_sti_nameservice_mode, spawned_sti_nameservice):
    if resolved_sti_nameservice_mode == "external":
        if not sti_nameservice:
            pytest.skip("pass --sti-nameservice host:port to use an external omniORB name service")
        yield sti_nameservice
        return

    if resolved_sti_nameservice_mode != "spawn":
        pytest.skip("unsupported STI name-service mode: {0}".format(resolved_sti_nameservice_mode))

    yield spawned_sti_nameservice


@pytest.fixture
def stipy_modules():
    stipy = pytest.importorskip("stipy")
    stidevicepy = pytest.importorskip("stipy.stidevicepy")
    return stipy, stidevicepy


@pytest.fixture
def observe_enabled(pytestconfig):
    return bool(pytestconfig.getoption("--observe"))


@pytest.fixture
def observe_timeout(pytestconfig):
    return float(pytestconfig.getoption("--observe-timeout"))


@pytest.fixture
def keep_network_alive(pytestconfig):
    return bool(pytestconfig.getoption("--keep-network-alive"))
