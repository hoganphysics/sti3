
%{
    #include <sti/engine/ParseResult.h>
    using STI::Engine::ParseResult;

    #include "ParsedDependencyTree.h"
    using STI::Engine::ParsedDependencyTree; 

    #include <sti/engine/EngineParsingMessage.h>
    using STI::Engine::EngineParsingMessage;

    #include <sti/engine/StackTraceResult.h>
    using STI::Engine::StackTraceResult;   
%}

%include "std_shared_ptr.i"
%include "std_vector.i"
%include "std_map.i"


%shared_ptr(STI::Engine::ParseResult);
%shared_ptr(STI::Engine::ParsedDependencyTree);
%shared_ptr(STI::Engine::StackTraceResult);


//EngineParsingMessage
%include "sti/engine/EngineParsingMessage.h"
%template(EngineParserMessageVector) std::vector< STI::Engine::EngineParsingMessage >;

%include "ParsedDependencyTree.h"
%include "sti/engine/StackTraceResult.h"
%include "sti/engine/ParseResult.h"
