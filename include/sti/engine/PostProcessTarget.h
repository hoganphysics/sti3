#ifndef STI_ENGINE_POSTPROCESSTARGET_H
#define STI_ENGINE_POSTPROCESSTARGET_H


#include <sti/engine/RawEventTargetDevice.h>

#include <string>


namespace STI
{
namespace Engine
{


//Symbolic address of a post-processing target on a (possibly remote) device.
//Mirrors RawEventTargetChannel's name-or-resolved-plus-abstract-flag shape, but
//is a distinct type so a ch()-built channel handle can never be passed where a
//post-processing target is expected (see docs/notes/postProcess.md, decision 5).
class PostProcessTarget
{
public:

    PostProcessTarget();
    PostProcessTarget(const RawEventTargetDevice& device, const std::string& name);
    PostProcessTarget(const std::string& deviceName, const std::string& name);

    bool isAbstract() const;          //true until resolved against the network
    const RawEventTargetDevice& device() const;
    std::string name() const;

    bool operator<(const PostProcessTarget& rhs) const;
    bool operator==(const PostProcessTarget& rhs) const;
    bool operator!=(const PostProcessTarget& rhs) const;

    template<class Archive>
    void serialize(Archive& archive);

private:

    RawEventTargetDevice _device;   //reuse as-is; device-name collisions are not a concern
    std::string _name;
    bool _isAbstract;
};


} //Engine
} //STI

#endif
