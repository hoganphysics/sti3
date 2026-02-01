
#include "PyResultTicket.h"

#include <sti/engine/ShotResult.h>
#include <sti/engine/ParseResult.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>


#include <sstream>

using STI::Python::PyResultTicket;

namespace py = pybind11;


void init_ResultTicket(py::module& m) 
{


    py::class_<PyResultTicket, std::shared_ptr<STI::Python::PyResultTicket>>(m, "ResultTicket")

        .def("wait", py::overload_cast<>(&STI::Python::PyResultTicket::wait, py::const_))
        .def("wait", py::overload_cast<const std::function<bool()>&>(&STI::Python::PyResultTicket::wait, py::const_))
        .def("cancel", &STI::Python::PyResultTicket::cancel)
        .def("defer", &STI::Python::PyResultTicket::defer)
        .def("status", &STI::Python::PyResultTicket::getStatus)
        .def("getShotID", &STI::Python::PyResultTicket::getShotID)
        .def("measurements", py::overload_cast<>(&STI::Python::PyResultTicket::measurements))
        .def("measurements", py::overload_cast<const STI::Device::DeviceID&>(&STI::Python::PyResultTicket::measurements))
        .def("measurements", py::overload_cast<const std::string&>(&STI::Python::PyResultTicket::measurements))
        .def("getParseResult", &STI::Python::PyResultTicket::getParseResult)
        .def("getShotResult", &STI::Python::PyResultTicket::getShotResult)
        .def("getMessages", &STI::Python::PyResultTicket::getMessages)
        .def("__repr__",
            [](const PyResultTicket& self) {
                std::stringstream s;
                // <ResultTicket | Complete | sid:user@machine#2022_05_12>
                s << "<ResultTicket | " << PyResultTicket::statusToString(self.getStatus()) << " | " << self.getShotID().print() << ">";
                return s.str();
            })
        ;

}
