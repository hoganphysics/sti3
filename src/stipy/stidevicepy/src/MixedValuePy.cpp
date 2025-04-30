
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

const STI::Utils::MixedValue& MixedValuePy::getMixedValue() const
{
    return (*this);
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
    
    //Empty, Boolean, Int, Double, String, Vector, VectorInt, Binary, File, Image, Any

    switch (value.getType())
    {
    case MixedValueType::Empty:
        obj = py::none();
        break;
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
                // pyList.append( convertValue(v) );
                pyList.append( MixedValuePy(v) );
            }

            obj = pyList;
        }
        break;
    case MixedValueType::VectorInt:
        {
            const std::vector<int>* values;
            if (value.getFlatVector(values)) {
                // obj = py::cast(*values);
                py::list pyList;
                for (auto& v : (*values)) {
                    pyList.append( py::cast(v) );
                }
                obj = pyList;
            }
        }
        break;
    case MixedValueType::Binary:
        {
            auto binaryData = value.getBinary();
            
            char* rawData;      //python bytes always uses char
            if (binaryData != 0 && binaryData->getBytes(rawData)) {
                obj = py::bytes(rawData, binaryData->bytes());  // Return the data without transcoding
            }
        }
        break;
    case MixedValueType::File:
        {
            
        }
        break;
    case MixedValueType::Image:
        {
            
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
    if (setValueExtract<py::bool_, bool>(value)) return;    //must be before int
    if (setValueExtract<py::int_, int>(value)) return;
    if (setValueExtract<py::str, std::string>(value)) return;

    if (value && py::isinstance<py::list>(value)) {
        const py::list& list_vals = value.cast<py::list>();
        for (const py::handle& obj : list_vals) {
            addValue_py(obj);
        }
    }

    if (value && py::isinstance<py::tuple>(value)) {
        const py::tuple& list_vals = value.cast<py::tuple>();
        for (const py::handle& obj : list_vals) {
            addValue_py(obj);
        }
    }
}

void MixedValuePy::addValue_py(const py::handle& value)
{
    if (addValueExtract<MixedValuePy, MixedValue>(value)) return;

    if (addValueExtract<py::float_, double>(value)) return;
    if (addValueExtract<py::bool_, bool>(value)) return;    //must be before int
    if (addValueExtract<py::int_, int>(value)) return;
    if (addValueExtract<py::str, std::string>(value)) return;  

    if (value && py::isinstance<py::list>(value)) {

        const py::list& list_vals = value.cast<py::list>();

        // if (isType(MixedValueType::Empty)) {
        //     //add list data to this level
        //     for (const py::handle& obj : list_vals) {
        //         addValue_py(obj);
        //     }
        // }
        // else {
        //     //Make and add new sublist
        //     MixedValuePy newListVal;

        //     for (const py::handle& obj : list_vals) {
        //         newListVal.addValue_py(obj);
        //     }
        //     addValue_py(newListVal);
        // }

        //Make and add new sublist
        MixedValuePy newListVal;

        for (const py::handle& obj : list_vals) {
            newListVal.addValue_py(obj);
        }
        addValue_py(newListVal);
    }

    if (value && py::isinstance<py::tuple>(value)) {

        const py::tuple& list_vals = value.cast<py::tuple>();

        //Make and add new sublist
        MixedValuePy newListVal;

        for (const py::handle& obj : list_vals) {
            newListVal.addValue_py(obj);
        }
        addValue_py(newListVal);
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

// pybind11::object MixedValuePy::flatten() const
// {
//     py::list nodes;

//     //DFS 

// }