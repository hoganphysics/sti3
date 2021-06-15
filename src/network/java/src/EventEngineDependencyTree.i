%feature("director");

%{
 
    #include "EventEngineDependencyTree.h"
    using STI::Engine::EventEngineDependencyTree;

    namespace boost {};

    #include "utils/DependencyTree.h"
    using STI::Utils::DependencyTree;   
%}

%shared_ptr(STI::Engine::EventEngineDependencyTree);

//EventEngineDependencyTree

namespace boost {};
%include "utils/DependencyTree.h"
%extend STI::Utils::DependencyTree< STI::Device::DeviceID > 
{
    int STI::Utils::DependencyTree< STI::Device::DeviceID >::getDependentNodeCount(const STI::Device::DeviceID& node) const
    {
        int count;
        self->getDependentNodeCount(node, count);
        return count;
    }

} 
%ignore STI::Utils::DependencyTree< STI::Device::DeviceID >::getDependentNodeCount(const STI::Device::DeviceID& node, int& count) const;
// %apply int& INOUT { int& count };


%shared_ptr(STI::Utils::DependencyTree< STI::Device::DeviceID >);
%template(DeviceIDDependencyTree) STI::Utils::DependencyTree< STI::Device::DeviceID >;

%include "EventEngineDependencyTree.h"
