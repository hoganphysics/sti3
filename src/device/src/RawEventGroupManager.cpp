
#include "RawEventGroupManager.h"

#include <sti/utils/utils.h>

#include <sstream>

using STI::Engine::RawEventGroupManager;
using STI::Engine::RawEventGroup;


RawEventGroupManager::RawEventGroupManager()
: STI::Utils::VectorMap<std::string, RawEventGroup>(), nextGroupIndex(0)
{
}

RawEventGroupManager::RawEventGroupManager(std::vector<RawEventGroup>& groups)
: STI::Utils::VectorMap<std::string, RawEventGroup>(groups), nextGroupIndex(0)
{
}

void RawEventGroupManager::addGroup(const std::string& name)
{
    if (exists(name)) return;

    //first character should be /, indicated abs path
    std::vector<std::string> names;
    STI::Utils::splitString(name, "/", names);

    //work backwards, from highest directory, checking if the group exists. 
    //if it exists, get the group and call group.addSubgroup(...). Then add the created subgroup.
    //if it doesn't exist, call addGroup() recursively.

    std::stringstream groupName;
    if (names.size() > 1) {
        // i=0 is "" since name begins with /
        // End at size()-1 since the last name is the new subgroup
        for (unsigned i = 0; i < names.size() - 1; ++i) {
            groupName << "/" << names.at(i);
        }
        
        //recursively make subgroups, if missing
        addGroup(groupName.str());

        RawEventGroup parentGroup;
        if (get(groupName.str(), parentGroup)) {
            
            RawEventGroup newGroup(names.back(), vec().size(), parentGroup.getFullIndex());
            add(name, newGroup);
            // nextGroupIndex = add(name, newGroup) + 1;
        }
    }
    else {
        RawEventGroup newGroup(names.back(), vec().size(), {});
        add(name, newGroup);
    }


}

bool RawEventGroupManager::getAbsGroupName(const RawEventGroup& group, std::string& absName) const
{
    auto indices = group.getFullIndex();

    std::stringstream absoluteName;

    for (auto& i : indices) {
        if (i >= vec().size()) {
            return false;
        }
        absoluteName << "/" << vec().at(i).getName();
    }

    absName = absoluteName.str();
    return exists(absName);
}

