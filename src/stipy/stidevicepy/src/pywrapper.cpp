
#include <sti/engine/Measurement.h>

#include <vector>
#include <memory>
#include <map>
#include <string>


#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

void init_EngineID(py::module& m);
// void init_Exception(py::module& m);
void init_DeviceMessage(py::module& m);
void init_DeviceMessageDispatcher(py::module& m);
void init_Channel(py::module& m);
void init_ChannelManager(py::module& m);
void init_SynchronousEvent(py::module& m);
void init_Device(py::module& m);
void init_LocalDevice(py::module& m);
void init_PartnerDevice(py::module& m);
void init_DeviceCollection(py::module& m);
void init_EventEngineScheduler(py::module& m);
void init_PersistenceManager(py::module& m);
void init_ProfileManager(py::module& m);
void init_LogManager(py::module& m);
void init_TaskManager(py::module& m);
void init_DeviceHub(py::module& m);


PYBIND11_MAKE_OPAQUE(std::vector<std::shared_ptr<STI::Engine::Measurement>>);
PYBIND11_MAKE_OPAQUE(std::map<std::string, std::string>);


PYBIND11_MODULE(stidevicepy, m) {
    m.doc() = "STI Device wrapper library";

    // py::module_::import("stidevicepybase");

    init_EngineID(m);
    // init_Exception(m);
    init_DeviceMessage(m);
    init_DeviceMessageDispatcher(m);
    init_Channel(m);
    init_ChannelManager(m);
    init_DeviceCollection(m);
    init_SynchronousEvent(m);
    init_Device(m);
    init_PartnerDevice(m);
    init_LocalDevice(m);
    init_EventEngineScheduler(m);
    init_PersistenceManager(m);
    init_ProfileManager(m);
    init_LogManager(m);
    init_TaskManager(m);
    init_DeviceHub(m);
}

int main(int argc, char *argv[])
{
    return 0;
}

