

#include "LocalDevice.h"
#include "DevicePy.h"
#include "LocalDevicePy.h"
#include "MixedValuePy.h"
#include "ChannelManagerPy.h"
#include "DeviceMessageDispatcher.h"

#include <memory>

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Python::DevicePy;
using STI::Python::LocalDevicePy;
using STI::Python::DevicePyTrampoline;
//using STI::Python::LocalDevicePyTrampoline;

using STI::Python::DevicePy2;
using STI::Python::LocalDevicePy2;
using STI::Python::LocalDevicePy2Trampoline;

using STI::Python::Animal2;
using STI::Python::PyAnimal2;
using STI::Python::Dog2;

int testDev(const std::shared_ptr<LocalDevicePy2>& dev) {
    return dev->test2(3);
}

void init_LocalDevice(py::module& m) 
{

    py::class_<Animal2, PyAnimal2 /* <--- trampoline*/>(m, "Animal2")
        .def(py::init<>())
        .def("go", &Animal2::go)
        .def("getID", &Animal2::getID);

    py::class_<Dog2, Animal2>(m, "Dog2")
        .def(py::init<>());



    py::class_<DevicePy2, std::shared_ptr<DevicePy2>>(m, "Device2")
    //    .def(py::init<const std::shared_ptr<STI::Device::Device>&>())
        .def(py::init<>())
        .def("getID", &DevicePy2::getID)
        .def("getDeviceCollection", &DevicePy2::getDeviceCollection)
        .def("getMessageDispatcher", &DevicePy2::getMessageDispatcher)
        .def("getChannelManager", &DevicePy2::getChannelManager)
        ;

    py::class_<LocalDevicePy2, DevicePy2, LocalDevicePy2Trampoline, std::shared_ptr<LocalDevicePy2>>(m, "LocalDevice2") 
        //.def(py::init<>())
         .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&>(), 
                     py::arg("name"), py::arg("address"), py::arg("module"), py::arg("targetServerID") )
         .def("writeChannel", &LocalDevicePy2::writeChannel)
         .def("readChannel", &LocalDevicePy2::readChannel)
         .def("addChannel", &LocalDevicePy2::addChannel)
         .def("addPartner", &LocalDevicePy2::addPartner)
         .def("test2", &LocalDevicePy2::test2);



    m.def("testDev", &testDev, "test device");


    py::class_<DevicePy, DevicePyTrampoline, std::shared_ptr<DevicePy>>(m, "Device")   //std::shared_ptr<DevicePy>
        .def(py::init<>())
        // .def("getID", &DevicePy::getIDpy)
        .def("getMessageDispatcher", &DevicePy::getMessageDispatcher)
        .def("getChannelManager", &DevicePy::getChannelManager)
        .def("getID", &DevicePy::getIDpy)
        .def("test2", &DevicePy::test2);

    py::class_<LocalDevicePy, DevicePy, std::shared_ptr<LocalDevicePy>>(m, "LocalDevice")  //std::shared_ptr<LocalDevicePy>//LocalDevicePyTrampoline
        //.def(py::init<>())
         .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&>(), 
                     py::arg("name"), py::arg("address"), py::arg("module"), py::arg("targetServerID") )
         //.def("test", &LocalDevicePy::test)
        // .def("writeChannel", &LocalDevicePy::writeChannelPy)
        ;

}

