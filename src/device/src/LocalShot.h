#ifndef STI_ENGINE_LOCALSHOT_H
#define STI_ENGINE_LOCALSHOT_H

#include "Shot.h"
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
    void getBaseEventGroup(std::shared_ptr<RawEventGroup>& baseGroup);
    void setBaseEventGroup(const std::shared_ptr<RawEventGroup>& baseGroup);

private:

    ShotConfig shotConfig;
    std::shared_ptr<RawEventGroup> baseEventGroup;
};


} //Engine
} //STI

#endif
