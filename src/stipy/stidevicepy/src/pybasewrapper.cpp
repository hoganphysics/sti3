
#include <sti/engine/Measurement.h>

#include "MixedValuePy.h"

#include <vector>
#include <memory>

#include <pybind11/pybind11.h>
namespace py = pybind11;

void init_DeviceID(pybind11::module&);
void init_FileID(py::module& m);
void init_MixedValue(py::module& m);
void init_StackTrace(py::module& m);
void init_RawEvent(py::module& m);
void init_HubID(py::module& m);
void init_Measurement(py::module& m);
void init_Attribute(py::module& m);
void init_AttributeManager(py::module& m);
void init_ShotConfig(py::module& m);
void init_Shot(py::module& m);
void init_ParseID(py::module& m);
void init_ShotID(py::module& m);
void init_Configuration(py::module& m);
void init_Sequence(py::module& m);
void init_ParsedDependencyTree(py::module& m);
void init_ParseResult(py::module& m);
void init_ShotResult(py::module& m);
void init_SequenceResult(py::module& m);
void init_Profile(py::module& m);
void init_LogID(py::module& m);
void init_LogRecord(py::module& m);
void init_Task(py::module& m);
void init_RawEventGroup(py::module& m);
void init_EngineParsingMessage(py::module& m);
void init_EnginePlayingMessage(py::module& m);
void init_FileServer(py::module& m);

PYBIND11_MAKE_OPAQUE(std::vector<std::shared_ptr<STI::Engine::Measurement>>);
PYBIND11_MAKE_OPAQUE(std::map<std::string, std::string>);
//PYBIND11_MAKE_OPAQUE(std::map<short, STI::Utils::MixedValue>);
PYBIND11_MAKE_OPAQUE(std::map<short, STI::Python::MixedValuePy>);

// PYBIND11_MAKE_OPAQUE(std::vector<std::shared_ptr<STI::Engine::Measurement>>);

PYBIND11_MODULE(stipybase, m) {
    m.doc() = "STI base wrapper library";

    // py::module_::import("stidevicepybase");

    init_DeviceID(m);
    init_ParsedDependencyTree(m);
    init_FileID(m);
    init_FileServer(m);
    init_MixedValue(m);
    init_StackTrace(m);
    init_RawEvent(m);
    init_RawEventGroup(m);
    init_HubID(m);
    init_Measurement(m);
    init_Attribute(m);
    init_AttributeManager(m);
    init_ShotConfig(m);
    init_Shot(m);
    init_ParseID(m);
    init_ShotID(m);
    init_Sequence(m);
    init_Configuration(m);
    init_ParseResult(m);
    init_ShotResult(m);
    init_SequenceResult(m);
    init_Profile(m);
    init_LogID(m);
    init_LogRecord(m);
    init_Task(m);
    init_EngineParsingMessage(m);
    init_EnginePlayingMessage(m);
}

int main(int argc, char* argv[])
{
    return 0;
}
