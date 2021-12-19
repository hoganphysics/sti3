

%{
    #include "JEventEngineScheduler.h"
    using STI::Engine::JEventEngineScheduler;
%}



//JEventEngineScheduler
%ignore STI::Engine::EventEngineScheduler;
%ignore STI::Engine::JEventEngineScheduler::JEventEngineScheduler(const std::shared_ptr< STI::Engine::EventEngineScheduler >& scheduler);
%include "JEventEngineScheduler.h"

