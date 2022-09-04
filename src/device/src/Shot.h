#ifndef STI_ENGINE_SHOT_H
#define STI_ENGINE_SHOT_H

#include <sti/utils/FileHolder.h>

#include <vector>
#include <memory>


namespace STI
{
namespace Engine
{

class RawEventGroup;
class ShotConfig;


class Shot
{
public:

    virtual ~Shot() {}

    virtual const ShotConfig& getShotConfig() const = 0;
    virtual void getBaseEventGroup(std::shared_ptr<RawEventGroup>& baseGroup) = 0;
};


} //Engine
} //STI

#endif
