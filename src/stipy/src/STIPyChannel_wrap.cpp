
#include "STIPyChannel.h"
#include "STIPyDevice.h"
#include <memory>

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_STIPyChannel(py::module& m) 
{

    py::class_<STI::Python::STIPyChannel, std::shared_ptr<STI::Python::STIPyChannel>>(m, "STIPyChannel")
        .def(py::init<const std::shared_ptr<STI::Python::STIPyDevice>&, unsigned>(), 
                        py::arg("dev"), py::arg("number") )
        //.def("getID", &STI::Network::HubID::id)
        
        ;

}

