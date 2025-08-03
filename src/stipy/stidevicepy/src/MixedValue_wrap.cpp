#include <sti/utils/MixedValue.h>
#include <sti/utils/FileID.h>
#include "MixedValuePy.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Python::MixedValuePy;


void init_MixedValue(py::module& m) 
{
    //MixedValueType { Empty, Boolean, Int, Double, String, Vector, VectorInt, Binary, File, Image, Any}

    py::enum_<MixedValueType>(m, "MixedValueType")
        .value("Empty", MixedValueType::Empty)
        .value("Boolean", MixedValueType::Boolean)
        .value("Int", MixedValueType::Int)
        .value("Double", MixedValueType::Double)
        .value("String", MixedValueType::String)
        .value("Vector", MixedValueType::Vector)
        .value("VectorInt", MixedValueType::VectorInt)
        .value("Binary", MixedValueType::Binary)
        .value("File", MixedValueType::File)
        .value("Image", MixedValueType::Image)
        .value("Number", MixedValueType::Number)
        .value("Any", MixedValueType::Any)
        ;

    py::class_<MixedValuePy>(m, "MixedValue")
        .def(py::init<>())
        .def(py::init<const py::object&>(), py::arg("value"))
        .def("getValue", &MixedValuePy::getValue_py)
        // .def("getValue",
        //     [](const MixedValuePy& val) {
        //         if (val.getType() == MixedValueType::File) {
        //             return val.getFileID();
        //         }
        //         return val.getValue_py();
        //     })
        .def("setValue", py::overload_cast<const MixedValuePy&>(&MixedValuePy::setValue_py), py::arg("MixedValue"))    //, py::keep_alive<1, 2>()
        .def("setValue", py::overload_cast<const py::object&>(&MixedValuePy::setValue_py), py::arg("value"))
        .def("addValue", py::overload_cast<const MixedValuePy&>(&MixedValuePy::addValue_py), py::arg("MixedValue"))
        .def("addValue", py::overload_cast<const py::handle&>(&MixedValuePy::addValue_py), py::arg("value"))
        .def("getType", &MixedValuePy::getType)
        .def("isType", py::overload_cast<const MixedValueType&>(&MixedValuePy::isType, py::const_), py::arg("type"))
        .def("isType", py::overload_cast<const std::vector<MixedValueType>&>(&MixedValuePy::isType, py::const_), py::arg("types"))
        .def("clear", &MixedValuePy::clear)
        .def("print", &MixedValuePy::print)
        .def("__len__",
            [](const MixedValuePy& val) {
                return val.getVector().size();
            })
        .def("__getitem__",
            [](const MixedValuePy& val, int index) {
                if (val.getType() != MixedValueType::Vector) {
                    throw py::type_error("Not a Vector");
                }
                if (index < 0) {
                    index += val.getVector().size();
                }
                if (index < 0 || index >= val.getVector().size()) {
                    throw py::index_error("Index out of range");
                }

                // return MixedValuePy::convertValue(val.getVector().at(index));
                return MixedValuePy(val.getVector().at(index));
            })
        // .def("__iter__",
        //     [](const MixedValuePy& val) 
        //     {
        //         if (val.getType() != MixedValueType::Vector) {
        //             throw py::type_error("Attempted to use an iterator on a MixedValue that is not a Vector");
        //         }
        //         // py::list pyList = val.getValue_py();
        //         // return py::make_iterator(val.getVector().begin(), val.getVector().end()); 
        //         // return py::make_iterator(pyList.begin(), pyList.end(), pyList);
        //     }, 
        //     py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
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

