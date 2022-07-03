#include <sti/LocalDeviceHub.h>
#include "DevicePy.h"
#include <sti/network/HubID.h>

#include <sti/NetworkDeviceHub.h>

#include <set>
#include <sstream>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/cast.h>

namespace py = pybind11;

using STI::Device::DeviceID;
using STI::Network::HubID;
using STI::Network::LocalDeviceHub;
using STI::Network::NetworkDeviceHub;

void init_DeviceHub(py::module& m) 
{

    // py::class_<LocalDeviceHub::HubNodeWalker::NodeType::DirectedGraphNodeType, 
    //             std::unique_ptr<LocalDeviceHub::HubNodeWalker::NodeType::DirectedGraphNodeType>>(m, "DeviceDirectedGraphNode")
    //     .def(py::init<>())
    //     .def_readonly("id", &LocalDeviceHub::HubNodeWalker::NodeType::DirectedGraphNodeType::id)
    //     .def_readonly("node", &LocalDeviceHub::HubNodeWalker::NodeType::DirectedGraphNodeType::node)
    //     .def_readonly("outConnections", &LocalDeviceHub::HubNodeWalker::NodeType::DirectedGraphNodeType::outConnections)
    //     ;

    // py::class_<LocalDeviceHub::HubNodeWalker::NodeType>(m, "DeviceDirectedGraphHub")
    //     .def(py::init<>())
    //     .def_readonly("id", &LocalDeviceHub::HubNodeWalker::NodeType::id)
    //     .def_readonly("nodes", &LocalDeviceHub::HubNodeWalker::NodeType::nodes)
    //     ;

    // py::class_<LocalDeviceHub::HubNodeWalker, std::unique_ptr<LocalDeviceHub::HubNodeWalker>>(m, "HubNodeWalker")
    //     .def(py::init<>())
    //     // .def_readonly("node", &LocalDeviceHub::HubNodeWalker::node)
    //     .def("node",[](LocalDeviceHub::HubNodeWalker& self) {
    //         return self.node;
    //     })
    //     .def_readonly("connections", &LocalDeviceHub::HubNodeWalker::connections)
    //     ;


     py::class_<LocalDeviceHub, std::shared_ptr<LocalDeviceHub>>(m, "LocalDeviceHub")
        // .def(py::init<const std::string&, const std::string&, unsigned short>(), 
        //                 py::arg("name"), py::arg("address"), py::arg("module") )
        .def(py::init<const HubID&>(), py::arg("hubID") )
        .def("addDevice",
            [](LocalDeviceHub& self, const std::shared_ptr<STI::Python::DevicePy>& devicePy)->bool {
                if (devicePy != 0) {
                    return self.addDevice(devicePy->getDevice());
                }
                return true;
            })
        .def("removeDevice", &LocalDeviceHub::removeDevice)
        .def("getDevice",
            [](LocalDeviceHub& self, const DeviceID& id) {
                std::shared_ptr<STI::Device::Device> device;

                if (self.getNode(id, device) && device != 0) {
                    auto devicePy = std::make_shared<STI::Python::DevicePy>(device);
                    return devicePy;
                }
                return std::make_shared<STI::Python::DevicePy>();
            })
        .def("getNodeIDs", [](const LocalDeviceHub& self){
            std::set<DeviceID> ids;
            self.getNodeIDs(ids);
            return ids;
        })
        .def("numberOfNodes", &LocalDeviceHub::numberOfNodes)
        .def("getID", &LocalDeviceHub::getID)
        .def("getHubIDs", [](const LocalDeviceHub& self){
            std::set<HubID> ids;
            self.getHubIDs(ids);
            return ids;
        })
        .def("containsHub", &LocalDeviceHub::containsHub)
        .def("disconnect", &LocalDeviceHub::disconnect)
        .def("__repr__",
            [](const LocalDeviceHub& hub) {
                std::stringstream s;
                s << "LocalDeviceHub (" << hub.getID().getID() << ") \n" 
                        << "Nodes: " << hub.numberOfNodes();
                return s.str();
            })
        ;
    
    ;
    

    m.def("connect", &LocalDeviceHub::connect, "Connect two hubs");


    py::class_<NetworkDeviceHub, std::shared_ptr<NetworkDeviceHub>>(m, "NetworkDeviceHub")
        .def(py::init<const std::string&>(), py::arg("nameServiceAddress") )
        .def(py::init<const std::string&, const STI::Utils::Configuration&>(), py::arg("nameServiceAddress"), py::arg("config") ) 
        .def(py::init<const STI::Utils::Configuration&>(), py::arg("config") ) 

        .def(py::init<const HubID&, const std::string&>(), py::arg("hubID"), py::arg("nameServiceAddress") )
        .def(py::init<const HubID&, const std::string&, const STI::Utils::Configuration&>(), 
                    py::arg("hubID"), py::arg("nameServiceAddress"), py::arg("config") )
        .def(py::init<const HubID&, const STI::Utils::Configuration&>(), py::arg("hubID"), py::arg("config") )

        // .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&>(), 
        //                 py::arg("name"), py::arg("address"), py::arg("module"), py::arg("nameServiceAddress") )
        .def("connect", &NetworkDeviceHub::connect)
        .def("addDevice", 
            [](NetworkDeviceHub& self, const std::shared_ptr<STI::Python::DevicePy>& devicepy) {
                if (devicepy != 0) {
                    return self.addDevice(devicepy->getDevice());
                }
                return false;
            })
        .def("getDeviceIDs", 
            [](const NetworkDeviceHub& self) {
                std::set<STI::Device::DeviceID> ids;
                self.getDeviceIDs(ids);
                return ids;
            })
        .def("run", py::overload_cast<bool>(&NetworkDeviceHub::run), py::arg("block") = true)
        .def("shutdown", &NetworkDeviceHub::shutdown)
        .def("printNetwork", py::overload_cast<>(&NetworkDeviceHub::printNetwork))
        .def("printNetwork", py::overload_cast<const std::string&>(&NetworkDeviceHub::printNetwork), py::arg("baseContext"))
        ;

}

