
#include "ResultTicket.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_ResultTicket(py::module& m) 
{


    py::class_<STI::Python::ResultTicket, std::shared_ptr<STI::Python::ResultTicket>>(m, "ResultTicket")

        .def("wait", &STI::Python::ResultTicket::wait)
        .def("cancel", &STI::Python::ResultTicket::cancel)
        ;

}
