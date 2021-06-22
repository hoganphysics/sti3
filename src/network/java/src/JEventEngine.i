
%{

    #include "JEventEngine.h"
    using STI::Engine::JEventEngine;

    #include "EngineState.h"
    using STI::Engine::EngineState;

%}

%shared_ptr(STI::Engine::JEventEngine);

%import "EventEngine.h"
%ignore STI::Engine::JEventEngine::JEventEngine(const std::shared_ptr< EventEngine >& engine);
%include "JEventEngine.h"

%include "EngineState.h"
