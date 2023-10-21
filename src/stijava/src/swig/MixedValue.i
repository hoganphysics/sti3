

%{
    #include <sti/utils/MixedValue.h>

    using STI::Utils::MixedValue;
    using STI::Utils::MixedValueType;
    using STI::Utils::MixedValueVector;
%}

%shared_ptr(STI::Utils::Image);

//MixedValue
%warnfilter(516) STI::Utils::MixedValue::setValue;
%include "sti/fwd/MixedValue_fwd.h"
%include "sti/utils/MixedValue.h"

%template(MixedValueVec) std::vector< STI::Utils::MixedValue >;
%include "sti/utils/MixedValue.h"
%rename(MixedValueVec) STI::Utils::MixedValueVector;

%template(MixedValueTypeVector) std::vector< STI::Utils::MixedValueType >;

%template(VectorInt) std::vector< int >;

%extend STI::Utils::MixedValue
{
    // MixedValue(const std::string& value)
    // {
    //     STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue();
    //     mixedVal->setValue(value);
    //     return mixedVal;
    // }
    // MixedValue(double value)
    // {
    //     STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue();
    //     mixedVal->setValue(value);
    //     return mixedVal;
    // }
    // MixedValue(int value)
    // {
    //     STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue();
    //     mixedVal->setValue(value);
    //     return mixedVal;
    // }
    // MixedValue(const std::shared_ptr< STI::Utils::BinaryData >& value)
    // {
    //     STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue();
    //     mixedVal->setValue(value);
    //     return mixedVal;
    // }
    // MixedValue(const std::shared_ptr< STI::Utils::FileHolder >& value)
    // {
    //     STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue();
    //     mixedVal->setValue(value);
    //     return mixedVal;
    // }
    // MixedValue(bool value)
    // {
    //     STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue();
    //     mixedVal->setValue(value);
    //     return mixedVal;
    // }

    void STI::Utils::MixedValue::setValueVector(const std::vector< STI::Utils::MixedValue >& value)
    {
        self->setValue< STI::Utils::MixedValue >(value);
    }
    void STI::Utils::MixedValue::addValue(const STI::Utils::MixedValue& value)
    {
        self->addValue< STI::Utils::MixedValue >(value);
    }

    const std::vector< int > STI::Utils::MixedValue::getVectorInt()
    {
        const std::vector< int >* values;
        if (self->getFlatVector< int >(values)) {
            return *values;
        }

        std::vector< int > empty;
        return empty;
    }
} 

