
#ifndef STI_PYTHON_MIXEDVALUEPY_H
#define STI_PYTHON_MIXEDVALUEPY_H

#include <sti/utils/MixedValue.h>

#include <pybind11/pybind11.h>


namespace STI
{
namespace Python
{


class MixedValuePy : public STI::Utils::MixedValue
{
public:

    MixedValuePy();
    MixedValuePy(const MixedValue& value);
    MixedValuePy(const pybind11::object& value);

    const STI::Utils::MixedValue& getMixedValue() const;
    STI::Utils::MixedValue& getMixedValue();
    pybind11::object getValue_py() const;

    void setValue_py(const pybind11::object& value);
    void addValue_py(const pybind11::handle& value);

    void setValue_py(const MixedValuePy& value);
    void addValue_py(const MixedValuePy& value);

    // pybind11::object flatten() const;

    static pybind11::object convertValue(const MixedValue& value);

private:

    template<typename PyT, typename T>
    bool setValueExtract(const pybind11::object& value)
    {
        bool success = false;

        try {
            if (value && pybind11::isinstance<PyT>(value)) {
                MixedValue::setValue( static_cast<T>(value.cast<PyT>()) );
                success = true;
            }
        }
        catch(pybind11::cast_error&) {
            success = false;
        }
        return success;
    }

    template<typename PyT, typename T>
    bool addValueExtract(const pybind11::handle& value)
    {
        bool success = false;

        try {
            if (value && pybind11::isinstance<PyT>(value)) {
                MixedValue::addValue( static_cast<T>(value.cast<PyT>()) );
                success = true;
            }
        }
        catch(pybind11::cast_error&) {
            success = false;
        }

        return success;
    }
    
};


} //Python
} //STI

#endif

