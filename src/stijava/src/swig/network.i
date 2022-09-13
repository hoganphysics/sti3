


%{
    #include "JNetworkDeviceHub.h"
    #include "JNodeWalker.h"
    #include <sti/network/HubID.h>
%}

//JNetworkDeviceHub
%include "JNetworkDeviceHub.h"

//HubID
%include "sti/network/HubID.h"

//JNodeWalker
%ignore STI::Network::JNodeWalker::JNodeWalker(STI::Network::LocalDeviceHub::HubNodeWalker& root);
%ignore STI::Network::JHubGraphNode::JHubGraphNode(const STI::Network::DirectedGraphHub< STI::Device::DeviceID, STI::Device::Device >& hub);
%ignore STI::Network::JDeviceGraphNode::JDeviceGraphNode(const STI::Network::DirectedGraphNode< STI::Device::DeviceID, STI::Device::Device >& deviceNode);
%include "JNodeWalker.h"
%template(JNodeWalkerVector) std::vector< STI::Network::JNodeWalker >;
%template(JDeviceGraphNodeVector) std::vector< STI::Network::JDeviceGraphNode >;

