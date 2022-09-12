
%{
    #include <sti/utils/GraphPathLabel.h>
    using STI::Utils::GraphPathLabel;

    #include <sti/utils/FileHolder.h>
    using STI::Utils::FileHolder;

    #include <sti/utils/MixedValue.h>

    using STI::Utils::MixedValue;
    using STI::Utils::MixedValueType;
    using STI::Utils::MixedValueVector;

    #include <sti/utils/MetaData.h>
    using STI::Utils::MetaData;
%}


%include "std_vector.i"
%include "std_string.i"
%include "std_shared_ptr.i"

%shared_ptr(STI::Utils::FileHolder);

%template(StringVector) std::vector< std::string >;

%template(UIntVector) std::vector< unsigned >;
%include "sti/utils/GraphPathLabel.h"
%rename(UIntVector) STI::Utils::GraphPathLabel;



//FileHolder
%template(FileHolderVector) std::vector< std::shared_ptr< STI::Utils::FileHolder > >;
%ignore STI::Utils::FileHolder::write(const char* buffer, unsigned length);
%ignore STI::Utils::FileHolder::openFile();
%ignore STI::Utils::FileHolder::closeFile();
%include "sti/utils/FileHolder.h"


//MixedValue
%warnfilter(516) STI::Utils::MixedValue::setValue;
%include "sti/fwd/MixedValue_fwd.h"


// %ignore STI::Utils::MixedValue::MixedValue;
%include "sti/utils/MixedValue.h"
%extend STI::Utils::MixedValue 
{
    MixedValue(const std::string& value)
    {
        STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue(value);
        return mixedVal;
    }
    MixedValue(double value)
    {
        STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue(value);
        return mixedVal;
    }
    MixedValue(int value)
    {
        STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue(value);
        return mixedVal;
    }
    MixedValue(const std::shared_ptr< STI::Utils::FileHolder >& value)
    {
        STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue(value);
        return mixedVal;
    }
    MixedValue(bool value)
    {
        STI::Utils::MixedValue* mixedVal = new STI::Utils::MixedValue(value);
        return mixedVal;
    }
}

%template(MixedValueVec) std::vector< STI::Utils::MixedValue >;
%include "sti/utils/MixedValue.h"
%rename(MixedValueVec) STI::Utils::MixedValueVector;

%include "sti/utils/MetaData.h"
