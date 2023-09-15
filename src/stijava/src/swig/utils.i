
%{
    #include <sti/utils/GraphPathLabel.h>
    using STI::Utils::GraphPathLabel;

%}


%include "std_vector.i"
%include "std_string.i"
%include "std_set.i"
%include "std_shared_ptr.i"



%template(StringVector) std::vector< std::string >;
%template(StringSet) std::set< std::string >;

%template(UIntVector) std::vector< unsigned >;
%include "sti/utils/GraphPathLabel.h"
%rename(UIntVector) STI::Utils::GraphPathLabel;


