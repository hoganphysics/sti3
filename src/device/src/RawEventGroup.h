#ifndef STI_ENGINE_RAWEVENTGROUP_H
#define STI_ENGINE_RAWEVENTGROUP_H


#include <sti/device/DeviceID.h>
#include <sti/utils/GraphPathLabel.h>

#include <string>
#include <memory>


namespace STI
{
namespace Engine
{

class RawEvent;
class RawEventGroup;


class RawEventGroup
{
public:

    RawEventGroup();
    RawEventGroup(const std::string& groupName, unsigned index);
    RawEventGroup(const std::string& groupName, unsigned index, const STI::Utils::GraphPathLabel& parentIndex);
    RawEventGroup(const std::string& groupName, const STI::Utils::GraphPathLabel& groupIndex);
    // RawEventGroup(const std::string& groupName, unsigned index, const STI::Engine::RawEventGroup& parent);

    bool operator==(const RawEventGroup& other) const;
    bool operator<(const RawEventGroup& other) const;

    std::string getName() const;

    double startTime() const;
    double endTime() const;

    void setStartTime(double start);
    void setEndTime(double end);

    void adjustTimeRange(const RawEvent& newEvent);

    unsigned getIndex() const;
    STI::Utils::GraphPathLabel getFullIndex() const;   //including parent group indices
    // STI::Utils::GraphPathLabel getParentGroupIndex() const;

    // STI::Device::DeviceID getTargetServerID() const;

    RawEventGroup makeSubgroup(const std::string& groupName, unsigned index);

    template<class Archive>
    void serialize(Archive& archive);

private:

    double t_start;
    double t_end;
    
    // unsigned index;
    STI::Utils::GraphPathLabel groupIndex;

    std::string name;
    // STI::Device::DeviceID targetServerID;

    // STI::Engine::RawEventGroup parentGroup;
};


} //Engine
} //STI

#endif
