#include "STIPyServer.h"

#include "STIPyShot.h"
#include "PyParseTicket.h"
#include "PyResultTicket.h"

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>


namespace py = pybind11;

using STI::Python::STIPyServer;
using STI::Python::STIPyShot;
using STI::Python::PyParseTicket;
using STI::Python::DevicePy;


void init_STIPyServer(py::module& m) 
{

    py::class_<STIPyServer, DevicePy, std::shared_ptr<STIPyServer>>(m, "STIPyServer")

        .def("makeshot", py::overload_cast<>(&STIPyServer::makeshot))
        .def("makeshot", py::overload_cast<const std::function<void(void)>&>(&STIPyServer::makeshot))
        .def("makeshot", py::overload_cast<const std::function<void(void)>&, const std::set<STI::Engine::ParsedVar>&>(&STIPyServer::makeshot))
        // .def("makesequence", py::overload_cast<>(&STIPyServer::makesequence))
        .def("parse", py::overload_cast<const std::shared_ptr<STIPyShot>&>(&STIPyServer::parse))
        .def("parse", py::overload_cast<const std::shared_ptr<STIPyShot>&, const STI::Engine::SequenceEntryID&>(&STIPyServer::parse))
        .def("play", py::overload_cast<const std::shared_ptr<PyParseTicket>&>(&STIPyServer::play))
        .def("parse", py::overload_cast<const std::shared_ptr<STI::Engine::Sequence>&>(&STIPyServer::parse))    //sequences

        .def("cancel_all", &STIPyServer::cancelAll)
        
        .def("setUsername", &STIPyServer::setUserName)
        .def("username", &STIPyServer::getUserName)
        .def("hub", &STIPyServer::getDeviceHub)
        
        .def("printNetwork", py::overload_cast<>(&STIPyServer::printNetwork))
        .def("printNetwork", py::overload_cast<const std::string&>(&STIPyServer::printNetwork))
        ;

}

