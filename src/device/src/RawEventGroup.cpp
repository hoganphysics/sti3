
#include "RawEventGroup.h"
#include <sti/engine/RawEvent.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

using STI::Engine::RawEventGroup;
using STI::Engine::RawEvent;


RawEventGroup::RawEventGroup()
: RawEventGroup("", 0, {})
{
}

RawEventGroup::RawEventGroup(const std::string& groupName, unsigned index)
: RawEventGroup(groupName, index, {})
{
}

RawEventGroup::RawEventGroup(const std::string& groupName, unsigned index, const STI::Utils::GraphPathLabel& parentIndex)
: RawEventGroup(groupName, parentIndex)
{
    groupIndex.push_back(index);
}

RawEventGroup::RawEventGroup(const std::string& groupName, const STI::Utils::GraphPathLabel& groupIndex)
: name(groupName), groupIndex(groupIndex)
{
}

bool RawEventGroup::operator==(const RawEventGroup& other) const
{
    return (getName() == other.getName() && getFullIndex() == other.getFullIndex());
}

bool RawEventGroup::operator<(const RawEventGroup& other) const
{
    if (getFullIndex() == other.getFullIndex()) {
        return getName() < other.getName();
    }
    return (getFullIndex() < other.getFullIndex());
}


// RawEventGroup::RawEventGroup(const std::string& groupName, unsigned index, const STI::Engine::RawEventGroup& parent)
// : RawEventGroup(groupName, index, parent.getFullIndex())
// {
// }

std::string RawEventGroup::getName() const
{
    return name;
}

double RawEventGroup::startTime() const
{
    return t_start;
}

double RawEventGroup::endTime() const
{
    return t_end;
}


void RawEventGroup::setStartTime(double start)
{
    t_start = start;
}

void RawEventGroup::setEndTime(double end)
{
    t_end = end;
}


void RawEventGroup::adjustTimeRange(const RawEvent& newEvent)
{
    if (newEvent.time() < t_start) {
        t_start = newEvent.time();
    }
    if (newEvent.time() > t_end) {
        t_end = newEvent.time();
    }

    // //recursively adjust all parent time ranges to fit the new child time range
    // if (parentGroup != 0) {
    //     parentGroup->adjustTimeRange(newEvent);
    // }
}


unsigned RawEventGroup::getIndex() const
{
    return groupIndex.back();
}

STI::Utils::GraphPathLabel RawEventGroup::getFullIndex() const
{
    // STI::Utils::GraphPathLabel fullIndex = parentGroupIndex;
    // fullIndex.push_back(index);
    return groupIndex;
}

// STI::Utils::GraphPathLabel RawEventGroup::getParentGroupIndex() const
// {
//     return parentGroupIndex;
// }


// STI::Device::DeviceID RawEventGroup::getTargetServerID() const
// {
//     return targetServerID;
// }

RawEventGroup RawEventGroup::makeSubgroup(const std::string& groupName, unsigned index)
{
    RawEventGroup subgroup(groupName, index, groupIndex);
    return subgroup;
}

template<class Archive>
void RawEventGroup::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("start time", t_start), 
		cereal::make_nvp("end time", t_end), 
		// cereal::make_nvp("index", index), 
		cereal::make_nvp("groupIndex", groupIndex),
		cereal::make_nvp("name", name)
		// cereal::make_nvp("targetServerID", targetServerID)
        // cereal::make_nvp("parentGroup", parentGroup)
		);
}


template void RawEventGroup::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void RawEventGroup::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

