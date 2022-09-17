
%{
    #include <sti/utils/FileHolder.h>
    using STI::Utils::FileHolder;
%}

%shared_ptr(STI::Utils::FileHolder);


//FileHolder
%template(FileHolderVector) std::vector< std::shared_ptr< STI::Utils::FileHolder > >;
%ignore STI::Utils::FileHolder::write(const char* buffer, unsigned length);
%ignore STI::Utils::FileHolder::openFile();
%ignore STI::Utils::FileHolder::closeFile();
%include "sti/utils/FileHolder.h"
