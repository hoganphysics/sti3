

#include <sti/utils/MixedValue.h>
#include "MixedValuePy.h"


#include <pybind11/pybind11.h>

namespace py = pybind11;

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Python::MixedValuePy;

void init_MixedValue(py::module& m) 
{
    //MixedValueType { Boolean, Int, Double, String, Vector, Empty, File, Image, Any}

    py::enum_<MixedValueType>(m, "MixedValueType")
        .value("Boolean", MixedValueType::Boolean)
        .value("Int", MixedValueType::Int)
        .value("Double", MixedValueType::Double)
        .value("String", MixedValueType::String)
        .value("Vector", MixedValueType::Vector)
        .value("Empty", MixedValueType::Empty)
        .value("File", MixedValueType::File)
        .value("Image", MixedValueType::Image)
        .value("Any", MixedValueType::Any)
        .export_values();

    py::class_<MixedValuePy>(m, "MixedValue")
        .def(py::init<>())
        .def(py::init<const py::object&>())
        .def("getValue", &MixedValuePy::getValue_py)
        .def("setValue", py::overload_cast<const MixedValuePy&>(&MixedValuePy::setValue_py), py::arg("MixedValue"))    //, py::keep_alive<1, 2>()
        .def("setValue", py::overload_cast<const py::object&>(&MixedValuePy::setValue_py), py::arg("value"))
        .def("addValue", py::overload_cast<const MixedValuePy&>(&MixedValuePy::addValue_py), py::arg("MixedValue"))
        .def("addValue", py::overload_cast<const py::handle&>(&MixedValuePy::addValue_py), py::arg("value"))
        .def("clear", &MixedValuePy::clear)
        .def("print", &MixedValuePy::print)
        .def("__repr__",
            [](const MixedValuePy& val) {
                return val.print();
            })
        .def("__eq__",  // operator ==
            [](const MixedValuePy& self, const MixedValuePy& other) {
                return ( static_cast<const MixedValue&>(self) == static_cast<const MixedValue&>(other) );
            })
        ;

}

