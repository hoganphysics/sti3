#ifndef STI_ENGINE_PARSEDDEPENDENCYTREE_H
#define STI_ENGINE_PARSEDDEPENDENCYTREE_H

#include <sti/device/DeviceID.h>

#include <vector>
#include <memory>

namespace STI
{
namespace Engine
{

class EventEngineDependencyTree;

struct DeviceIDVertex
{
    STI::Device::DeviceID id;
    std::vector<unsigned> outConnections;

    template<class Archive>
    void serialize(Archive& archive);
};


class ParsedDependencyTree
{
public:

    ParsedDependencyTree();
    ParsedDependencyTree(const std::shared_ptr<EventEngineDependencyTree>& tree);

    void getNodes(std::vector<STI::Device::DeviceID>& nodes) const;
    void getDependedentNodes(const STI::Device::DeviceID& node, std::vector<STI::Device::DeviceID>& depNodes) const;

    template<class Archive>
    void serialize(Archive& archive);

private:

    std::vector<DeviceIDVertex> vertices;

};


} //Engine
} //STI

#endif
