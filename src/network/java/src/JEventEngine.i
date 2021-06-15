
%{

    #include "JEventEngine.h"
    using STI::Engine::JEventEngine;

    #include "EngineState.h"
    using STI::Engine::EngineState;

%}

%shared_ptr(STI::Engine::JEventEngine);

%import "EventEngine.h"
%include "JEventEngine.h"

%include "EngineState.h"
