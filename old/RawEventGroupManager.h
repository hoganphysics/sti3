#ifndef STI_ENGINE_RAWEVENTGROUPMANAGER_H
#define STI_ENGINE_RAWEVENTGROUPMANAGER_H


#include <sti/utils/VectorMap.h>

#include <sti/engine/RawEventGroup.h>

#include <string>
#include <vector>


namespace STI
{
namespace Engine
{

class RawEvent;
class RawEventGroup;


class RawEventGroupManager : public STI::Utils::VectorMap<std::string, RawEventGroup> 
{
public:

    RawEventGroupManager();
    RawEventGroupManager(std::vector<RawEventGroup>& groups);

    void addGroup(const std::string& name);

    bool getAbsGroupName(const RawEventGroup& group, std::string& absName) const;

private:

    unsigned nextGroupIndex;
};

} //Engine
} //STI

#endif
