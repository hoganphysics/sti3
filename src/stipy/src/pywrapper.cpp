

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_DeviceID(py::module &);
void init_stipy(py::module& m);

void init_HubID(py::module& m);
void init_STIPyDevice(py::module& m);
void init_STIPyChannel(py::module& m);
void init_STIPyShot(py::module& m);
void init_STIPyServer(py::module& m);
void init_ParseTicket(py::module& m);
void init_ResultTicket(py::module& m);
// void init_DeviceID(py::module& m);

PYBIND11_MODULE(stipy, m) {

    py::module_::import("stidevicepy");
    //py::module_::import("stidevicepybase");

    m.doc() = "STIPy interface allows parsing and playing of timing files on the STI network"; // module docstring


    // init_HubID(m);
    // init_DeviceID(m);

    init_stipy(m);

    init_STIPyDevice(m);
    init_STIPyChannel(m);
    init_STIPyShot(m);
    init_STIPyServer(m);

    init_ParseTicket(m);
    init_ResultTicket(m);

    //init_DeviceID(m);

}

int main(int argc, char *argv[])
{
    return 0;
}

