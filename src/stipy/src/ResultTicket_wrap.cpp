
#include "PyResultTicket.h"

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

namespace py = pybind11;


void init_ResultTicket(py::module& m) 
{


    py::class_<STI::Python::PyResultTicket, std::shared_ptr<STI::Python::PyResultTicket>>(m, "ResultTicket")

        .def("wait", py::overload_cast<>(&STI::Python::PyResultTicket::wait))
        .def("wait", py::overload_cast<const std::function<bool()>&>(&STI::Python::PyResultTicket::wait))
        .def("cancel", &STI::Python::PyResultTicket::cancel)
        .def("getStatus", &STI::Python::PyResultTicket::getStatus)
        ;

}
