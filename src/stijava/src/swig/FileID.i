%{

    #include <sti/utils/FileID.h>
    using STI::Utils::FileID;

%}

%ignore STI::Utils::FileID::commonBasePath(const std::vector< STI::Utils::FileID >& fileIDs);



%include "sti/utils/FileID.h"

%template(FileIDVector) std::vector< STI::Utils::FileID >;
