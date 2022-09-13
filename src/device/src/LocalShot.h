#ifndef STI_ENGINE_LOCALSHOT_H
#define STI_ENGINE_LOCALSHOT_H

#include <sti/engine/Shot.h>
#include <sti/engine/ShotConfig.h>

#include <memory>
#include <string>


namespace STI
{
namespace Engine
{

class RawEvent;
class RawEventGroup;


class LocalShot : public Shot
{
public:

    LocalShot(const ShotConfig& config, const std::shared_ptr<RawEventGroup>& baseGroup);
    virtual ~LocalShot();

    const ShotConfig& getShotConfig() const;
    void getRootEventGroup(std::shared_ptr<RawEventGroup>& rootGroup);
    void setRootEventGroup(const std::shared_ptr<RawEventGroup>& rootGroup);

private:

    ShotConfig shotConfig;
    std::shared_ptr<RawEventGroup> rootEventGroup;
};


} //Engine
} //STI

#endif
