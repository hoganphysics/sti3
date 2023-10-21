%{

    #include <sti/utils/FileID.h>
    using STI::Utils::FileID;

%}

%template(FileIDVector) std::vector< STI::Utils::FileID >;

%ignore STI::Utils::FileID::commonBasePath;

%include "sti/utils/FileID.h"

