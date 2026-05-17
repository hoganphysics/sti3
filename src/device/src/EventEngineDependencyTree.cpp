
#include "EventEngineDependencyTree.h"

#include <vector>

using STI::Engine::EventEngineDependencyTree;
using STI::Device::DeviceID;


EventEngineDependencyTree::EventEngineDependencyTree() : STI::Utils::DependencyTree<STI::Device::DeviceID>() 
{
}

bool EventEngineDependencyTree::hasBranchToTarget(const DeviceID& root, const DeviceID& target) const
{
    DeviceID branch;
    return getBranchToTarget(root, target, branch);
}

bool EventEngineDependencyTree::hasGraphBranchToTarget(const DeviceID& root, const DeviceID& target) const
{
    DeviceID branch;
    return getGraphBranchToTarget(root, target, branch);
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

/// Find the node connected to root that leads to target, following graph edges directly.
bool EventEngineDependencyTree::getGraphBranchToTarget(const DeviceID& root, const DeviceID& target, DeviceID& branch) const
{
    if(!hasVertex(target) || !hasVertex(root)) {
        return false;
    }
    std::vector<DeviceID> parentNodes;
    getParentNodes(target, parentNodes);

    for(auto& parent : parentNodes) {
        if(parent == root) {
            branch = target;
            return true;
        }
        else if (getGraphBranchToTarget(root, parent, branch)) {
            return true;
        }
    }

    return false;
}
