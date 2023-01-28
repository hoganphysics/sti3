
%{
    #include <sti/engine/ParseResult.h>
    using STI::Engine::ParseResult;

    #include <sti/engine/ParsedDependencyTree.h>
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


%immutable STI::Engine::StackTraceResult::pid;
%immutable STI::Engine::StackTraceResult::stackTraceData;
%shared_ptr(STI::Engine::StackTraceResult);


//EngineParsingMessage
%include "sti/engine/EngineParsingMessage.h"
%template(EngineParserMessageVector) std::vector< STI::Engine::EngineParsingMessage >;



%extend STI::Engine::ParsedDependencyTree
{
    std::vector< STI::Device::DeviceID > getNodes() const
    {
        std::vector< STI::Device::DeviceID > nodes;
        self->getNodes(nodes);
        return nodes;
    }

    std::vector< STI::Device::DeviceID > getDependedentNodes(const STI::Device::DeviceID& node) const
    {
        std::vector< STI::Device::DeviceID > depNodes;
        self->getDependedentNodes(node, depNodes);
        return depNodes;
    }
}
%ignore STI::Engine::ParsedDependencyTree::getNodes(std::vector< STI::Device::DeviceID >& nodes) const;
%ignore STI::Engine::ParsedDependencyTree::getDependedentNodes(const STI::Device::DeviceID& node, std::vector< STI::Device::DeviceID >& depNodes) const;


%include <sti/engine/ParsedDependencyTree.h>
%include "sti/engine/StackTraceResult.h"

%immutable STI::Engine::ParseResult::pid;
%immutable STI::Engine::ParseResult::baseEventGroup;
%immutable STI::Engine::ParseResult::parsedDevices;
%immutable STI::Engine::ParseResult::messages;
%immutable STI::Engine::ParseResult::stackTraceResult;


%include "sti/engine/ParseResult.h"
