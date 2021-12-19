
%{
    #include "JEventEngineJob.h"
    using STI::Engine::JEventEngineJob;
    #include "EventEngineJob.h"
    using STI::Engine::EventEngineJob;

    #include "EngineJobStatus.h"
    using STI::Engine::EngineJobStatus;

    // #include "JEventEngine.h"
    // using STI::Engine::JEventEngine;
%}

%shared_ptr(STI::Engine::JEventEngineJob);

%include "EngineJobStatus.h"

//JEventEngineJob
%ignore STI::Engine::EventEngineJob;
%ignore STI::Engine::JEventEngineJob::JEventEngineJob(const std::shared_ptr< EventEngineJob >& eventEngineJob);
%template(JEventEngineJobVector) std::vector< std::shared_ptr< STI::Engine::JEventEngineJob > >;
%include "JEventEngineJob.h"

