

#include "STIPyServer.h"
#include "STIPyShot.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_STIPyServer(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<STI::Python::STIPyServer, std::shared_ptr<STI::Python::STIPyServer>>(m, "STIPyServer")

        .def("makeshot", py::overload_cast<>(&STI::Python::STIPyServer::makeshot))
        ;

}

