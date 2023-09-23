#include "DeviceCollectionPy.h"
#include "DevicePy.h"

#include <set>
#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
namespace py = pybind11;

using STI::Python::DeviceCollectionPy;


void init_DeviceCollection(py::module& m) 
{

    py::class_<DeviceCollectionPy, std::shared_ptr<DeviceCollectionPy>>(m, "DeviceCollection")
        .def("contains", &DeviceCollectionPy::contains, py::arg("deviceID"))
        .def("size", &DeviceCollectionPy::size)
        .def("get", &DeviceCollectionPy::get, py::arg("deviceID"))
        .def("getIDs", &DeviceCollectionPy::getIDs)
        .def("clear", &DeviceCollectionPy::clear)
        .def("add", &DeviceCollectionPy::add, py::arg("deviceID"), py::arg("Device"))
        .def("remove", &DeviceCollectionPy::remove, py::arg("deviceID"))
        ;

}

