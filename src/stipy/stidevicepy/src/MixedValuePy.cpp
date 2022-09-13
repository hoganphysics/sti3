
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

STI::Utils::MixedValue& MixedValuePy::getMixedValue()
{
    return (*this);
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
    if (setValueExtract<MixedValuePy, MixedValue>(value)) return;

    if (setValueExtract<py::float_, double>(value)) return;
    if (setValueExtract<py::int_, int>(value)) return;
    if (setValueExtract<py::str, std::string>(value)) return;
    if (setValueExtract<py::bool_, bool>(value)) return;

    if (value && py::isinstance<py::list>(value)) {
        const py::list& list_vals = value.cast<py::list>();
        for (const py::handle& obj : list_vals) {
            addValue_py(obj);
        }
    }
}

void MixedValuePy::addValue_py(const py::handle& value)
{
    if (addValueExtract<MixedValuePy, MixedValue>(value)) return;

    if (addValueExtract<py::float_, double>(value)) return;
    if (addValueExtract<py::int_, int>(value)) return;
    if (addValueExtract<py::str, std::string>(value)) return;
    if (addValueExtract<py::bool_, bool>(value)) return;

    if (value && py::isinstance<py::list>(value)) {

        const py::list& list_vals = value.cast<py::list>();

        if (isType(MixedValueType::Empty)) {
            //add list data to this level
            for (const py::handle& obj : list_vals) {
                addValue_py(obj);
            }
        }
        else {
            //Make and add new sublist
            MixedValuePy newListVal;

            for (const py::handle& obj : list_vals) {
                newListVal.addValue_py(obj);
            }
            addValue_py(newListVal);
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
