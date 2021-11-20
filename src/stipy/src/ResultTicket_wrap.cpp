
#include "PyResultTicket.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_ResultTicket(py::module& m) 
{


    py::class_<STI::Python::PyResultTicket, std::shared_ptr<STI::Python::PyResultTicket>>(m, "ResultTicket")

        .def("wait", &STI::Python::PyResultTicket::wait)
        .def("cancel", &STI::Python::PyResultTicket::cancel)
        .def("getStatus", &STI::Python::PyResultTicket::getStatus)
        ;

}
