%feature("director");

%{
    #include <sti/device/DeviceIDIndexedGraph.h>
    using STI::Device::DeviceIDIndexedGraph;

    #include "utils/IndexedGraph.h"
    using STI::Utils::IndexedGraph;   
%}

%include "std_vector.i"

//DeviceIDIndexedGraph

%include "utils/IndexedGraph.h"

%template(DeviceIDIndexedGraphNode) STI::Utils::IndexedGraphNode< STI::Device::DeviceID >;

%template(DeviceIDIndexedGraphNodeVector) std::vector< STI::Utils::IndexedGraphNode< STI::Device::DeviceID > >;

%template(DeviceIDIndexedGraph) STI::Utils::IndexedGraph< STI::Device::DeviceID >;

%include "sti/device/DeviceIDIndexedGraph.h"
