

#include "STIPyServer.h"
#include "STIPyShot.h"
#include "ParseTicket.h"
#include "ResultTicket.h"

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

namespace py = pybind11;

using STI::Python::STIPyServer;
using STI::Python::STIPyShot;
using STI::Python::ParseTicket;

void init_STIPyServer(py::module& m) 
{

    py::class_<STIPyServer, std::shared_ptr<STIPyServer>>(m, "STIPyServer")

        .def("makeshot", py::overload_cast<>(&STIPyServer::makeshot))
        .def("makeshot", py::overload_cast<const std::function<void(void)>&>(&STIPyServer::makeshot))
        .def("parse", py::overload_cast<const std::shared_ptr<STIPyShot>&>(&STIPyServer::parse))
        .def("play", py::overload_cast<const std::shared_ptr<ParseTicket>&>(&STIPyServer::play))
        .def("cancelAll", &STIPyServer::cancelAll)
        ;

}

