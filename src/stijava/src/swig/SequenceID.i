
%{
    #include <sti/engine/SequenceID.h>
    using STI::Engine::SequenceID;
    using STI::Engine::SequenceIndex;
    using STI::Engine::SequenceEntryID;

    #include <sti/engine/EngineJobStatus.h>
    using STI::Engine::EngineJobStatus;

    #include <sti/engine/TimeStamp.h>
    using STI::Engine::TimeStamp;

%}

%include "std_shared_ptr.i"
%include "std_vector.i"
%include "std_map.i"


%template(SequenceEngineJobStatusMap) std::map< STI::Engine::SequenceIndex, STI::Engine::EngineJobStatus, std::less< STI::Engine::SequenceIndex > >;
typedef std::map< STI::Engine::SequenceIndex, STI::Engine::EngineJobStatus, std::less< STI::Engine::SequenceIndex > >::iterator SequenceEngineJobStatusMapIterator;


//SequenceID
%include "sti/engine/SequenceID.h"



