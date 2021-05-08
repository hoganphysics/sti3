

#include "LocalDevice.h"
#include "DevicePy.h"
#include "LocalDevicePy.h"
#include "MixedValuePy.h"
#include "ChannelManagerPy.h"
#include "DeviceMessageDispatcher.h"
#include "LocalAttribute.h"
#include "EventEngineSchedulerPy.h"
#include "AttributeManagerPy.h"


#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

//using STI::Python::DevicePy;
//using STI::Python::LocalDevicePy;
//using STI::Python::DevicePyTrampoline;
//using STI::Python::LocalDevicePyTrampoline;

using STI::Python::DevicePy;
using STI::Python::LocalDevicePy;
using STI::Python::LocalDevicePyTrampoline;

// using STI::Python::Animal2;
// using STI::Python::PyAnimal2;
//using STI::Python::Dog2;

// int testDev(const std::shared_ptr<LocalDevicePy2>& dev) {
//     return dev->test2(3);
// }

void init_LocalDevice(py::module& m) 
{

    // py::class_<Animal2, PyAnimal2 /* <--- trampoline*/>(m, "Animal2")
    //     .def(py::init<>())
    //     .def("go", &Animal2::go)
    //     .def("getID", &Animal2::getID);

    // py::class_<Dog2, Animal2>(m, "Dog2")
    //     .def(py::init<>());



    py::class_<DevicePy, std::shared_ptr<DevicePy>>(m, "Device")
    //    .def(py::init<const std::shared_ptr<STI::Device::Device>&>())
        .def(py::init<>())
        .def("getID", &DevicePy::getID)
        .def("getDeviceCollection", &DevicePy::getDeviceCollection)
        .def("getEngineScheduler", &DevicePy::getEngineScheduler)
        .def("getMessageDispatcher", &DevicePy::getMessageDispatcher)
        .def("getChannelManager", &DevicePy::getChannelManager)
        .def("getAttributeManager", &DevicePy::getAttributeManager)
        ;

    py::class_<LocalDevicePy, DevicePy, LocalDevicePyTrampoline, std::shared_ptr<LocalDevicePy>>(m, "LocalDevice") 
        //.def(py::init<>())
         .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&>(), 
                     py::arg("name"), py::arg("address"), py::arg("module"), py::arg("targetServerID") )
         .def("writeChannel", &LocalDevicePy::writeChannel)
         .def("readChannel", &LocalDevicePy::readChannel)
         .def("addChannel", &LocalDevicePy::addChannel)
         .def("addPartner", &LocalDevicePy::addPartner)
        //  .def("addAttribute", &LocalDevicePy::addAttribute)
         .def("addAttribute", 
                py::overload_cast<const std::string&, const std::string&>(&LocalDevicePy::addAttribute), 
                py::return_value_policy::reference)
        //  .def("addAttribute", py::overload_cast<const std::string&, const std::string&, const pybind11::list&>(&LocalDevicePy::addAttribute))
         .def("addAttribute", 
                py::overload_cast<const std::string&, const std::string&, const std::vector<std::string>&>(&LocalDevicePy::addAttribute), 
                py::return_value_policy::reference)
         .def("test2", &LocalDevicePy::test2)
         ;



    //m.def("testDev", &testDev, "test device");


    // py::class_<DevicePy, DevicePyTrampoline, std::shared_ptr<DevicePy>>(m, "Device")   //std::shared_ptr<DevicePy>
    //     .def(py::init<>())
    //     // .def("getID", &DevicePy::getIDpy)
    //     .def("getMessageDispatcher", &DevicePy::getMessageDispatcher)
    //     .def("getChannelManager", &DevicePy::getChannelManager)
    //     .def("getID", &DevicePy::getIDpy)
    //     .def("test2", &DevicePy::test2);

    // py::class_<LocalDevicePy, DevicePy, std::shared_ptr<LocalDevicePy>>(m, "LocalDevice")  //std::shared_ptr<LocalDevicePy>//LocalDevicePyTrampoline
    //     //.def(py::init<>())
    //      .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&>(), 
    //                  py::arg("name"), py::arg("address"), py::arg("module"), py::arg("targetServerID") )
    //      //.def("test", &LocalDevicePy::test)
    //     // .def("writeChannel", &LocalDevicePy::writeChannelPy)
    //     ;

}

