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

    std::vector< STI::Device::DeviceID > getParentNodes(const STI::Device::DeviceID& node) const
    {
        std::vector< STI::Device::DeviceID > parentNodes;
        self->getParentNodes(node, parentNodes);
        return parentNodes;
    }

    std::vector< STI::Device::DeviceID > getCycle() const
    {
        std::vector< STI::Device::DeviceID > cycle;
        self->getCycle(cycle);
        return cycle;
    }

    std::vector< STI::Device::DeviceID > sortTree()
    {
        std::vector< STI::Device::DeviceID > orderedNodes;
        self->sortTree(orderedNodes);
        return orderedNodes;
    }

    STI::Utils::DependencyTree< STI::Device::DeviceID > getSubtree(const STI::Device::DeviceID& vertex)
    {
        STI::Utils::DependencyTree< STI::Device::DeviceID > tree;
        self->getSubtree(vertex, tree);
        return tree;
    }

}
%ignore STI::Utils::DependencyTree< STI::Device::DeviceID >::getDependentNodeCount(const STI::Device::DeviceID& node, int& count) const;
%ignore STI::Utils::DependencyTree< STI::Device::DeviceID >::getNodes(std::vector< STI::Device::DeviceID >& nodes) const;
%ignore STI::Utils::DependencyTree< STI::Device::DeviceID >::getNodes(std::set< STI::Device::DeviceID >& nodes) const;
%ignore STI::Utils::DependencyTree< STI::Device::DeviceID >::getDependedentNodes(const STI::Device::DeviceID& node, std::vector< STI::Device::DeviceID >& depNodes) const;
%ignore STI::Utils::DependencyTree< STI::Device::DeviceID >::getParentNodes(const STI::Device::DeviceID& node, std::vector< STI::Device::DeviceID >& parentNodes) const;
%ignore STI::Utils::DependencyTree< STI::Device::DeviceID >::getCycle(std::vector< STI::Device::DeviceID >& cycle) const;
%ignore STI::Utils::DependencyTree< STI::Device::DeviceID >::sortTree(std::vector< STI::Device::DeviceID >& orderedNodes);
%ignore STI::Utils::DependencyTree< STI::Device::DeviceID >::getSubtree(const STI::Device::DeviceID& vertex, STI::Utils::DependencyTree< STI::Device::DeviceID >& tree);

// %apply int& INOUT { int& count };

%extend STI::Engine::EventEngineDependencyTree
{
    STI::Device::DeviceID getBranchToTarget(const STI::Device::DeviceID& root, const STI::Device::DeviceID& target) const
    {
        STI::Device::DeviceID branch;
        self->getBranchToTarget(root, target, branch);
        return branch;
    }

}
%ignore STI::Engine::EventEngineDependencyTree::getBranchToTarget(const STI::Device::DeviceID& root, const STI::Device::DeviceID& target, STI::Device::DeviceID& branch) const;


%shared_ptr(STI::Utils::DependencyTree< STI::Device::DeviceID >);
%template(DeviceIDDependencyTree) STI::Utils::DependencyTree< STI::Device::DeviceID >;

%include "EventEngineDependencyTree.h"
