#ifndef STI_ENGINE_RAWEVENTGROUP_H
#define STI_ENGINE_RAWEVENTGROUP_H

#include <sti/fwd/RawEvent_fwd.h>
#include <sti/fwd/MixedValue_fwd.h>
#include <sti/engine/ParsedTag.h>
#include <sti/engine/ParsedVar.h>
#include <sti/utils/VectorMap.h>
#include <sti/utils/MetaData.h>


#include <string>
#include <memory>
#include <map>
#include <vector>
#include <set>
#include <mutex>


namespace STI
{
namespace Engine
{

class ParsedVar;
class ParsedTag;
class RawEvent;
class RawEventGroup;
class RawEventTarget;
class RawEventTargetDevice;
class StackTrace;
class StackTraceData;


struct RawEventGroupStats
{
    unsigned vars;
    unsigned tags;
    unsigned events;
    unsigned subgroups;

    RawEventGroupStats& operator+=(const RawEventGroupStats& rhs)
    {
        vars += rhs.vars;
        tags += rhs.tags;
        events += rhs.events;
        subgroups += rhs.subgroups;
        return (*this);
    }
};


class RawEventGroup
{

public:

    RawEventGroup();
    RawEventGroup(const std::string& name, const std::string& parentName);
    RawEventGroup(const std::string& name, const std::string& parentName, const std::shared_ptr<StackTraceData>& traceData);

    std::string getName() const;
    std::string getFullName() const;
    std::string getParentGroupName() const;
    double startTime() const;
    double endTime() const;

    RawEventGroupStats getStats() const;
    RawEventGroupStats getTotalStats() const;

    void setName(const std::string& newName);

    bool operator==(const RawEventGroup& other) const;
    bool operator<(const RawEventGroup& other) const;

    struct Result {
        bool success;
        std::string errorMessage;
    };

    Result addvar(const std::string& fullVarName, const STI::Utils::MixedValue& value, 
                const StackTrace& stackTrace);  //checks overwritten list and uses the overwritten value if found; fails if already bound and not in overwritten (cannnot call setvar twice)
    Result addtag(const std::string& fullTagName, const StackTrace& stackTrace);

    void addEvent(const RawEvent& evt);
    void addEvent(const RawEvent& evt, const std::string& subgroupName);

    void addEvent(const RawEventTarget& target, double time, const STI::Utils::MixedValue& value, 
                    const RawEventType& type, const StackTrace& stackTrace);

    void addEvent(const RawEventTarget& target, double time, const STI::Engine::ParsedVar& var, 
                    const RawEventType& type, const StackTrace& stackTrace);

    void addEvent(const RawEventTarget& target, double time, const STI::Utils::MixedValue& value, 
                    const RawEventType& type);

    void addEvents(const RawEventVector& newEvents);

    ParsedVar var(const std::string& fullVarName, const StackTrace& stackTrace);   //the value of the var, or an unbound var

    //for overwritten vars
    Result bindVar(const std::string& fullVarName, const STI::Utils::MixedValue& value);
    Result bindVar(const ParsedVar& overwrittenVar);
    Result bindVars(const std::set<ParsedVar>& overwritten);   //fails if it attempts to overwrite any already bound var
    //sequence overwritten vars must be declared as an argument to makeshot, so that python parsing can account for them
    //It's not possible to bindVars after addEvent, since in general the added events can depend on the initial bound values

    void bindTargets(const std::map<std::string, RawEventTarget>& targetReplacements);
    void bindDeviceTargets(const std::map<std::string, RawEventTargetDevice>& targetDeviceReplacements);

    double getTimeOffset() const;
    void shiftStartTimeTo(double time); //shifts all events so that group start is at time
    void shiftEndTimeTo(double time); //shifts all events so that group end is at time
    void shiftReferenceTimeTo(const std::string& refName, double time);   //shifts all events so that this reference point is at time

    void addReferencePoint(const std::string& refName, double time);   //adds a new named reference point
    bool getReferencePoint(const std::string& refName, double& time) const;
    std::map<std::string, double> getReferencePoints() const;

    std::shared_ptr<RawEventGroup> group(const std::string& groupName);
    std::vector<std::shared_ptr<RawEventGroup>> getSubgroups() const;
    void addSubgroup(const std::shared_ptr<RawEventGroup>& subgroup);

    void merge(const RawEventGroup& other);
    void swapEvents(RawEventGroup& other);
    void copyEvents(const RawEventGroup& other);

    void sortEvents();

    void clear();

    bool eventsEmpty() const;

    std::shared_ptr<StackTraceData> getStackTraceData() const;
    std::shared_ptr<RawEventVector> getEvents() const;    //should these reflect timeOrigin? no
    std::vector<ParsedVar> getVars() const;
    std::vector<ParsedTag> getTags() const;
    
    std::set<ParsedVar> getOverwrittenVars() const;

    void setVars(const std::vector<ParsedVar>& vars);
    void setTags(const std::vector<ParsedTag>& tags);

    bool getConcreteTarget(const RawEventTarget& abstractTarget, RawEventTarget& concreteTarget) const;

    RawEventGroup& addMetaData(const std::string& key, const STI::Utils::MixedValue& data);
    RawEventGroup& addMetaData(const STI::Utils::MetaData& data);
    const STI::Utils::MetaData& getMetaData() const;
    STI::Utils::MixedValue getMetaData(const std::string& key) const;




    template<class Archive>
    void serialize(Archive& archive);

private:

    void setParentName(const std::string& newParentName);
    void _addEvent(RawEvent& evt);
    void refreshMinMax();
    
    std::string name;   //name of group relative to parent
    std::string parentName;     //full parent group name

    double timeOffset;   //tracks all time shifts
    double timeMin;     //smallest added event time
    double timeMax;     //largest added event time

    std::map<std::string, double> referencePoints;
    unsigned eventNumber;

    std::shared_ptr<RawEventVector> events;

    std::shared_ptr<StackTraceData> stackTraceData; //global for shot

    STI::Utils::MetaData metaData;
    
    //specialize targets and vars in each group instance (deep copy)
    std::map<std::string, RawEventTarget> targets;   //replace abstract targets in event list
    std::map<std::string, RawEventTargetDevice> targetDevices;   //replace abstract devices in event list
    std::set<ParsedVar> overwrittenVars;

    std::vector<ParsedVar> parsedVars;
    std::vector<ParsedTag> parsedTags; 
    std::vector<std::shared_ptr<RawEventGroup>> subgroups;

    typedef STI::Utils::VectorMap<std::string, std::shared_ptr<RawEventGroup>> VectorMapRawEventGroup;
    VectorMapRawEventGroup groupMap;

    typedef STI::Utils::VectorMap<std::string, STI::Engine::ParsedVar> VectorMapParsedVar;
    VectorMapParsedVar varMap;

    typedef STI::Utils::VectorMap<std::string, STI::Engine::ParsedTag> VectorMapParsedTag;
    VectorMapParsedTag tagMap;

    static std::string getBaseGroup(const std::string& groupName);
    static void splitGroupName(const std::string& groupName, std::string& baseName, std::string& subName);
    static bool splitFullGroupName(const std::string& fullName, std::string& groupName, std::string& leafName);

    mutable std::mutex groupMutex;

};


} //Engine
} //STI

#endif
