
%{
    #include <sti/engine/Sequence.h>
    using STI::Engine::Sequence;
    using STI::Engine::SequenceType;
    using STI::Engine::SequenceEntry;
    using STI::Engine::SequenceIndex;

    #include <sti/engine/SequenceResult.h>
    using STI::Engine::SequenceResult;

    #include <sti/engine/EngineJobStatus.h>
    using STI::Engine::EngineJobStatus;
%}

%include "std_shared_ptr.i"
%include "std_vector.i"
%include "std_map.i"


%shared_ptr(STI::Engine::Sequence);
%shared_ptr(STI::Engine::SequenceResult);



//Sequence
%template(SequenceEntryMap) std::map< STI::Engine::SequenceIndex, STI::Engine::SequenceEntry >;
typedef std::map< STI::Engine::SequenceIndex, STI::Engine::SequenceEntry >::iterator SequenceEntryMapIterator;


%include "sti/engine/Sequence.h"


//SequenceResult
// Important: This template definition must appear earlier; it now is in SequenceID.i
// %template(SequenceEngineJobStatusMap) std::map< STI::Engine::SequenceIndex, STI::Engine::EngineJobStatus, std::less< STI::Engine::SequenceIndex > >;
// typedef std::map< STI::Engine::SequenceIndex, STI::Engine::EngineJobStatus, std::less< STI::Engine::SequenceIndex > >::iterator SequenceEngineJobStatusMapIterator;


%template(SequenceShotIDMap) std::map< STI::Engine::SequenceIndex, STI::Engine::ShotID, std::less< STI::Engine::SequenceIndex > >;
typedef std::map< STI::Engine::SequenceIndex, STI::Engine::ShotID, std::less< STI::Engine::SequenceIndex > >::iterator SequenceShotIDMapIterator;

%include "sti/engine/SequenceResult.h"

