#include <sti/engine/RawEvent.h>

#include <catch2/catch_test_macros.hpp>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace sti3_test
{

void callPythonWithRawEventMapAfterLocalDevicePyHeader(
    const py::function& callback,
    const STI::Engine::RawEventMap& events);

} // namespace sti3_test

TEST_CASE("LocalDevicePy parseEvents trampoline can pass RawEventMap to Python", "[stidevicepy][parseevents]")
{
    static py::scoped_interpreter guard{};

    STI::Engine::RawEventMap events;
    events.emplace(12.5, STI::Engine::RawEventVector{});

    py::list captured;
    py::function captureEvents = py::cpp_function([&captured](const py::object& received) {
        captured.append(received);
    });

    REQUIRE_NOTHROW(sti3_test::callPythonWithRawEventMapAfterLocalDevicePyHeader(captureEvents, events));

    REQUIRE(py::len(captured) == 1);
    REQUIRE(py::isinstance<py::dict>(captured[0]));

    py::dict received = py::reinterpret_borrow<py::dict>(captured[0]);
    REQUIRE(py::len(received) == 1);
    REQUIRE(received.contains(py::float_(12.5)));
    CHECK(py::len(received[py::float_(12.5)]) == 0);
}
