#include "STIPyServer.h"

#include <sti/engine/ShotConfig.h>

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

        .def("makeshot", py::overload_cast<const STI::Engine::ShotType&>(&STIPyServer::makeshot), py::arg("shotType"))
        .def("makeshot", py::overload_cast<const std::function<void(void)>&, const STI::Engine::ShotType&>(&STIPyServer::makeshot), py::arg("func"), py::arg("shotType"))
        .def("makeshot", py::overload_cast<const std::function<void(void)>&, const std::set<STI::Engine::ParsedVar>&, const STI::Engine::ShotType&>(&STIPyServer::makeshot), py::arg("func"), py::arg("vars"), py::arg("shotType"))
        // .def("makesequence", py::overload_cast<>(&STIPyServer::makesequence))
        
        .def("parse", py::overload_cast<const std::shared_ptr<STIPyShot>&>(&STIPyServer::parse), py::arg("shot"))
        .def("parse", py::overload_cast<const std::shared_ptr<STIPyShot>&, const STI::Engine::SequenceID&>(&STIPyServer::parse), py::arg("shot"), py::arg("sequenceID"))
        .def("parse", py::overload_cast<const std::shared_ptr<STIPyShot>&, const STI::Engine::SequenceEntryID&>(&STIPyServer::parse), py::arg("shot"), py::arg("sequenceEntryID"))

        .def("addSequence", py::overload_cast<const std::shared_ptr<STI::Engine::Sequence>&>(&STIPyServer::addSequence), py::arg("sequence"))
        .def("closeSequence", &STIPyServer::closeSequence)
        .def("cancelSequence", &STIPyServer::cancelSequence)

        .def("play", py::overload_cast<const std::shared_ptr<PyParseTicket>&>(&STIPyServer::play), py::arg("ticket"))
        .def("play", py::overload_cast<const STI::Engine::ParseID&>(&STIPyServer::play), py::arg("parseID"))

        .def("cancel_all", &STIPyServer::cancelAll)
        
        .def("setUsername", &STIPyServer::setUserName, py::arg("name"))
        .def("username", &STIPyServer::getUserName)
        .def("setHostname", &STIPyServer::setHostname, py::arg("name"))
        .def("hostname", &STIPyServer::getHostname)
        .def("hub", &STIPyServer::getDeviceHub)
        
        .def("printNetwork", py::overload_cast<>(&STIPyServer::printNetwork))
        .def("printNetwork", py::overload_cast<const std::string&>(&STIPyServer::printNetwork))
        ;

}

