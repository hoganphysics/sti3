
#include "EventEngineDependencyTree.h"

#include <vector>

using STI::Engine::EventEngineDependencyTree;
using STI::Device::DeviceID;

EventEngineDependencyTree::EventEngineDependencyTree() : STI::Utils::DependencyTree<STI::Device::DeviceID>() 
{
}

/// Find the node connected to root that leads to target, via each node's target server.
bool EventEngineDependencyTree::getBranchToTarget(const DeviceID& root, const DeviceID& target, DeviceID& branch) const
{
    if(!hasVertex(target) || !hasVertex(root)) {
        return false;
    }
    std::vector<DeviceID> parentNodes;
    getParentNodes(target, parentNodes);

    //find server of each node recursively until root is reached
    for(auto& parent : parentNodes) {
        if(parent.getID() == target.getTargetServerID()) {
            
            if(parent == root) {     //exit condition
                branch = target;    //This is the node we're looking for
                return true;
            }
            else {
                return getBranchToTarget(root, parent, branch);
            }
        }
    }

    return false;
}
