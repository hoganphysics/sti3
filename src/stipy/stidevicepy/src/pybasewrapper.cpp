

#include <pybind11/pybind11.h>
namespace py = pybind11;


void init_DeviceID(pybind11::module&);
void init_MixedValue(py::module& m);
void init_RawEvent(py::module& m);
void init_HubID(py::module& m);

void init_ShotConfig(py::module& m);
void init_ParseID(py::module& m);
void init_Configuration(py::module& m);

PYBIND11_MODULE(stipybase, m) {
    m.doc() = "STI base wrapper library";

    // py::module_::import("stidevicepybase");

    init_DeviceID(m);
    init_MixedValue(m);
    init_RawEvent(m);
    init_HubID(m);

    init_ShotConfig(m);
    init_ParseID(m);
    
    init_Configuration(m);
}

int main(int argc, char* argv[])
{
    return 0;
}

