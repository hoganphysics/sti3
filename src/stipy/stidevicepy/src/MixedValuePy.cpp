
#include "MixedValuePy.h"

using STI::Python::MixedValuePy;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace py = pybind11;


MixedValuePy::MixedValuePy()
: MixedValue()
{
}

MixedValuePy::MixedValuePy(const MixedValue& value)
: MixedValue(value)
{
}


//MixedValuePy::MixedValuePy(const MixedValuePy& value)
////: MixedValue(static_cast<MixedValue>(value))
//: MixedValue(value)
//{
//
//}

MixedValuePy::MixedValuePy(const py::object& value)
: MixedValue()
{
    setValue_py(value);
}


pybind11::object MixedValuePy::getValue_py() const
{
    return convertValue(*this);
}

pybind11::object MixedValuePy::convertValue(const MixedValue& value)
{
    py::object obj = py::none();
    
    //Boolean, Int, Double, String, Vector, Empty, File, Image, Any

    switch (value.getType())
    {
    case MixedValueType::Boolean:
        obj = py::cast( value.getBoolean() );
        break;
    case MixedValueType::Int:
        obj = py::cast( value.getInt() );
        break;
    case MixedValueType::Double:
        obj = py::cast( value.getDouble() );
        break;
    case MixedValueType::String:
        obj = py::cast( value.getString() );
        break;
    case MixedValueType::Vector:
        {
            py::list pyList;
            const STI::Utils::MixedValueVector& vec = value.getVector();

            for (auto& v : vec) {
                pyList.append( convertValue(v) );
            }

            obj = pyList;
        }
        break;
    default:
        break;
    }     

    return obj;
}

void MixedValuePy::setValue_py(const py::object& value)
{
//    pybind11::list;

    if (setValueExtract<MixedValuePy, MixedValuePy>(value)) return;

    if (setValueExtract<py::float_, double>(value)) return;
    if (setValueExtract<py::int_, int>(value)) return;
    if (setValueExtract<py::str, std::string>(value)) return;
    if (setValueExtract<py::bool_, bool>(value)) return;

    // if (pybind11::isinstance<pybind11::int_>(value)) {
    //     MixedValue::setValue( value.cast<int>() );
    // }

    if (value && py::isinstance<py::list>(value)) {
        const py::list& list_vals = value.cast<py::list>();
        for (const py::handle& obj : list_vals) {
            addValue_py(obj);
        }
    }
}

void MixedValuePy::addValue_py(const py::handle& value)
{
    if (addValueExtract<py::float_, double>(value)) return;
    if (addValueExtract<py::int_, int>(value)) return;
    if (addValueExtract<py::str, std::string>(value)) return;
    if (addValueExtract<py::bool_, bool>(value)) return;

    if (value && py::isinstance<py::list>(value)) {
        
        const py::list& list_vals = value.cast<py::list>();
        
        for (const py::handle& obj : list_vals) {
            addValue_py(obj);
        }
    }
}


void MixedValuePy::setValue_py(const MixedValuePy& value)
{
    const MixedValue& v = static_cast<const MixedValue&>(value);

    MixedValue::setValue(v);
}

void MixedValuePy::addValue_py(const MixedValuePy& value)
{
    MixedValue::addValue( static_cast<const MixedValue&>(value) );
}
