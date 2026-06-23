#include "AttributeManagerPy.h"
#include <sti/device/AttributeManager.h>
#include <sti/device/Attribute.h>

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
namespace py = pybind11;

using STI::Device::Attribute;
using STI::Python::AttributeManagerPy;
using STI::Device::AttributeManager;


void init_AttributeManager(py::module& m) 
{

    py::class_<AttributeManagerPy, std::shared_ptr<AttributeManagerPy>>(m, "AttributeManager")
        .def("getValue", &AttributeManagerPy::getValue, py::arg("key"), py::call_guard<py::gil_scoped_release>())
        .def("setValue", &AttributeManagerPy::setValue, py::arg("key"), py::arg("value"))
        .def("refreshValue", &AttributeManagerPy::refreshValue, py::arg("key"))
        .def("refreshValues", &AttributeManagerPy::refreshValues)
        .def("getAttribute", py::overload_cast<const std::string&>(&AttributeManagerPy::getAttribute), py::arg("key"))
        .def("getAttributes", py::overload_cast<>(&AttributeManagerPy::getAttributes))
        ;
}

