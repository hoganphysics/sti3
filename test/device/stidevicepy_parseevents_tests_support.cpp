#include "LocalDevicePy.h"

#include <sti/fwd/RawEvent_fwd.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace sti3_test
{

void callPythonWithRawEventMapAfterLocalDevicePyHeader(
    const py::function& callback,
    const STI::Engine::RawEventMap& events)
{
    callback(events);
}

} // namespace sti3_test
