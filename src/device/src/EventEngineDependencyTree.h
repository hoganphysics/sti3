#ifndef STI_ENGINE_EVENTENGINEDEPENDENCYTREE_H
#define STI_ENGINE_EVENTENGINEDEPENDENCYTREE_H

#include "utils/DependencyTree.h"
#include <sti/device/DeviceID.h>

namespace STI
{
namespace Engine
{


class EventEngineDependencyTree : public STI::Utils::DependencyTree<STI::Device::DeviceID>
{
public:

    EventEngineDependencyTree();

    //Find the node connected to root that leads to target, via each node's target server.
    bool getBranchToTarget(const STI::Device::DeviceID& root, const STI::Device::DeviceID& target, STI::Device::DeviceID& branch) const;
    bool hasBranchToTarget(const STI::Device::DeviceID& root, const STI::Device::DeviceID& target) const;

    //Find the node connected to root that leads to target, following graph edges directly.
    //This ignores targetServerID and should be used for dependency-graph reachability checks,
    //not for runtime event routing decisions.
    bool getGraphBranchToTarget(const STI::Device::DeviceID& root, const STI::Device::DeviceID& target, STI::Device::DeviceID& branch) const;
    bool hasGraphBranchToTarget(const STI::Device::DeviceID& root, const STI::Device::DeviceID& target) const;

};


} //Engine
} //STI

#endif
