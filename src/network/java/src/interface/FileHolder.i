
%{
    #include "utils/FileHolder.h"
    using STI::Utils::FileHolder;
%}

%shared_ptr(STI::Utils::FileHolder);


//FileHolder
%ignore STI::Utils::FileHolder::write(const char* buffer, unsigned length);
%ignore STI::Utils::FileHolder::openFile();
%ignore STI::Utils::FileHolder::closeFile();
%template(FileHolderVector) std::vector< std::shared_ptr< STI::Utils::FileHolder > >;
%include "utils/FileHolder.h"
