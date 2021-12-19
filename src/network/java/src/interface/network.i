


%{
    #include "JNetworkDeviceHub.h"
    #include "JNodeWalker.h"
    #include "HubID.h"
%}

//JNetworkDeviceHub
%include "JNetworkDeviceHub.h"

//HubID
%include "HubID.h"

//JNodeWalker
%ignore STI::Network::JNodeWalker::JNodeWalker(STI::Network::LocalDeviceHub::HubNodeWalker& root);
%ignore STI::Network::JHubGraphNode::JHubGraphNode(const STI::Network::DirectedGraphHub< STI::Device::DeviceID, STI::Device::Device >& hub);
%ignore STI::Network::JDeviceGraphNode::JDeviceGraphNode(const STI::Network::DirectedGraphNode< STI::Device::DeviceID, STI::Device::Device >& deviceNode);
%include "JNodeWalker.h"
%template(JNodeWalkerVector) std::vector< STI::Network::JNodeWalker >;
%template(JDeviceGraphNodeVector) std::vector< STI::Network::JDeviceGraphNode >;

