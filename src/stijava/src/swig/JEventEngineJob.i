
%{
    #include "JEventEngineJob.h"
    using STI::Engine::JEventEngineJob;
    #include <sti/engine/EventEngineJob.h>
    using STI::Engine::EventEngineJob;

    #include <sti/engine/EngineJobStatus.h>
    using STI::Engine::EngineJobStatus;

    // #include "JEventEngine.h"
    // using STI::Engine::JEventEngine;
%}

%shared_ptr(STI::Engine::JEventEngineJob);

%include "sti/engine/EngineJobStatus.h"

//JEventEngineJob
%ignore STI::Engine::EventEngineJob;
%ignore STI::Engine::JEventEngineJob::JEventEngineJob(const std::shared_ptr< EventEngineJob >& eventEngineJob);
%include "JEventEngineJob.h"

