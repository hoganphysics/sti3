
#include "ParsedDependencyTree.h"
#include "EventEngineDependencyTree.h"

#include <algorithm>

using STI::Engine::ParsedDependencyTree;


ParsedDependencyTree::ParsedDependencyTree(const std::shared_ptr<EventEngineDependencyTree>& tree)
{
    if (tree == 0) return;
    
    std::vector<STI::Device::DeviceID> nodes;
    std::vector<STI::Device::DeviceID> dependents;
    tree->getNodes(nodes);

    std::map<STI::Device::DeviceID, unsigned> indexmap;

    for (unsigned i = 0; i < nodes.size(); ++i) {
        indexmap[nodes.at(i)] = i;
    }


    for (auto& id : nodes) {
        dependents.clear();
        tree->getDependedentNodes(id, dependents);

        DeviceIDVertex vertex;
        vertex.id = id;

        for (auto& dep : dependents) {
            vertex.outConnections.push_back(indexmap[id]);
        }

        vertices.push_back(vertex);
    }

}

void ParsedDependencyTree::getNodes(std::vector<STI::Device::DeviceID>& nodes) const
{
    for (auto& vertex : vertices) {
        nodes.push_back(vertex.id);
    }
}

void ParsedDependencyTree::getDependedentNodes(const STI::Device::DeviceID& node, std::vector<STI::Device::DeviceID>& depNodes) const
{
    auto it = std::find_if(vertices.begin(), vertices.end(), [node](auto& vertex){ return vertex.id == node; });
    
    if (it == vertices.end()) return;

    for (auto& index : it->outConnections) {
        if (index >= 0 && index < vertices.size()) {
            depNodes.push_back(vertices.at(index).id);
        }
    }
}

