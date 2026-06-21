#include <sti/engine/PostProcessTarget.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>

using STI::Engine::PostProcessTarget;
using STI::Engine::RawEventTargetDevice;


PostProcessTarget::PostProcessTarget()
: _device(), _name(""), _isAbstract(true)
{
}

PostProcessTarget::PostProcessTarget(const RawEventTargetDevice& device, const std::string& name)
: _device(device), _name(name), _isAbstract(device.isAbstract())
{
}

PostProcessTarget::PostProcessTarget(const std::string& deviceName, const std::string& name)
: _device(deviceName), _name(name), _isAbstract(true)
{
}

bool PostProcessTarget::isAbstract() const
{
    return _isAbstract;
}

const RawEventTargetDevice& PostProcessTarget::device() const
{
    return _device;
}

std::string PostProcessTarget::name() const
{
    return _name;
}

bool PostProcessTarget::operator<(const PostProcessTarget& rhs) const
{
    if (_device == rhs._device) {
        return _name < rhs._name;
    }
    return _device < rhs._device;
}

bool PostProcessTarget::operator==(const PostProcessTarget& rhs) const
{
    return _device == rhs._device && _name == rhs._name;
}

bool PostProcessTarget::operator!=(const PostProcessTarget& rhs) const
{
    return !((*this) == rhs);
}

template<class Archive>
void PostProcessTarget::serialize(Archive& archive)
{
    archive(
        cereal::make_nvp("device", _device),
        cereal::make_nvp("name", _name),
        cereal::make_nvp("isAbstract", _isAbstract)
        );
}

template void PostProcessTarget::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void PostProcessTarget::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
