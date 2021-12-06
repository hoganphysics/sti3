

#include <pybind11/pybind11.h>
namespace py = pybind11;


void init_DeviceID(pybind11::module &);
void init_EngineID(py::module& m) ;
void init_DeviceMessage(py::module& m);
void init_DeviceMessageDispatcher(py::module& m);
void init_Channel(py::module& m);
void init_MixedValue(py::module& m);
void init_ChannelManager(py::module& m);
void init_SynchronousEvent(py::module& m);
void init_LocalDevice(py::module& m);
void init_DeviceCollection(py::module& m);
void init_EventEngineScheduler(py::module& m);
void init_Attribute(py::module& m);
void init_RawEvent(py::module& m);
void init_AttributeManager(py::module& m);
void init_PersistenceManager(py::module& m);



PYBIND11_MODULE(stidevicepy, m) {
    m.doc() = "STI Device wrapper library";

    // py::module_::import("stidevicepybase");

    init_DeviceID(m);
    init_EngineID(m);
    init_DeviceMessage(m);
    init_DeviceMessageDispatcher(m);
    init_Channel(m);
    init_MixedValue(m);
    init_ChannelManager(m);
    init_DeviceCollection(m);
    init_SynchronousEvent(m);
    init_LocalDevice(m);
    init_RawEvent(m);
    init_EventEngineScheduler(m);
    init_Attribute(m);
    init_AttributeManager(m);
    init_PersistenceManager(m);
}

int main(int argc, char *argv[])
{
    return 0;
}

