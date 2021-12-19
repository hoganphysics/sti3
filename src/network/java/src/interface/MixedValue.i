

%{
    #include "MixedValue.h"

    using STI::Utils::MixedValue;
    using STI::Utils::MixedValueType;
    using STI::Utils::MixedValueVector;
%}


//MixedValue
%warnfilter(516) STI::Utils::MixedValue::setValue;
%include "fwd/MixedValue_fwd.h"
%include "MixedValue.h"
%template(MixedValueVec) std::vector< STI::Utils::MixedValue >;
%include "MixedValue.h"
%rename(MixedValueVec) STI::Utils::MixedValueVector;

%extend STI::Utils::MixedValue
{
    void STI::Utils::MixedValue::setValueVector(const std::vector< STI::Utils::MixedValue >& value)
    {
        self->setValue< STI::Utils::MixedValue >(value);
    }
    void STI::Utils::MixedValue::addValue(const STI::Utils::MixedValue& value)
    {
        self->addValue< STI::Utils::MixedValue >(value);
    }
} 

