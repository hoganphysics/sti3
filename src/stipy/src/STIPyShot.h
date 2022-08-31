
#ifndef STI_PYTHON_STIPYSHOT_H
#define STI_PYTHON_STIPYSHOT_H

#include <sti/fwd/RawEvent_fwd.h>

#include "LocalShot.h"
#include <sti/device/DeviceID.h>
#include <sti/engine/StackTrace.h>
#include "RawEventGroup.h"
#include <sti/engine/RawEventTarget.h>
// #include "RawEventGroupManager.h"

#include <sti/utils/VectorMap.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileHolderFactory.h>

#include "RawStackTrace.h"

#include <pybind11/pybind11.h>

#include <vector>
#include <memory>
#include <mutex>
#include <map>

namespace STI
{
namespace Python
{

class STIPyServer;
class ParseTicket;
class MixedValuePy;
// class ParsedVarPy;

/*

g = group("MOT")
g2 = g.subgroup("Shutter")

g2 = group("MOT/Shutter")

g2.open()
g2.close()

frame1 = dev("Frame 1")
frame1.makegroup("MOT")

motGroup = makegroup("MOT")
motGroup.subgroup("Shutter")
motGroup.makegroup("Shutter")
g = makegroup("MOT/Shutter")


event(dds, 3.4, (95.2, 100, 0), motGroup)
event(dds, 3.4, (95.2, 100, 0), "Blue MOT/Shutter")

"Frame 1/Blue MOT/Shutter"

"Frame 1|Blue MOT|Shutter"

"<Frame 1/localhost/0>/Blue MOT/Shutter"


setvar("x", 23)
getvar("x")     #returns 23
var("y")        #dynamic value?

event(var("t"), ch2, 3.3, group2)   #requires { var("t") : 26 } at parse


//Affine?
var("t") + 5
3*var("t")

var("t") + var("t2")    //?

*/


// class StackTraceData
// {
// public:
    
//     StackTraceData();
//     //needed mutex locks

//     StackTrace addStackTrace(const RawStackTrace& stackTrace);
//     RawStackTrace getStackTrace(const StackTrace& stackTrace) const;

//     std::vector<std::shared_ptr<STI::Utils::FileHolder>> getTimingFiles() const;
//     // std::vector<std::string> timingFileNames()
//     std::vector<std::string> getFunctionNames() const;

// private:
    
//     unsigned addFile(const std::string& filename);

//     std::vector<std::shared_ptr<STI::Utils::FileHolder>> timingFiles;
//     // std::vector<std::string> timingFileNames; 
//     std::vector<std::string> functionNames;

//     typedef STI::Utils::VectorMap<std::string, std::string> VectorMapString;
//     std::shared_ptr<VectorMapString> functionMap;

//     typedef STI::Utils::VectorMap<std::string, std::shared_ptr<STI::Utils::FileHolder>> VectorMapFileHolder;
//     std::shared_ptr<VectorMapFileHolder> fileMap;
// };

// class RawEventGroup
// {

// public:

//     RawEventGroup(const std::string& name);

//     std::string getName() const;

//     bool addvar(const std::string& name, const MixedValue& value, 
//                 const STI::Engine::RawStackTrace& stackTrace);  //checks overwritten list and uses the overwritten value if found; fails if already bound and not in overwritten (cannnot call setvar twice)
//     bool addtag(const std::string& name, const STI::Engine::RawStackTrace& stackTrace);

//     void addEvent(const STI::Engine::RawEventTarget& target, double time, const MixedValue& value, 
//                     const STI::Engine::RawEventType& type, const RawStackTrace& stackTrace);

//     // pybind11::object getvar(const std::string& name);

//     ParsedVar var(const std::string& name);   //the value of the var, or an unbound var

//     bool bindVars(const std::vector<ParsedVar>& overwritten);   //fails if it attempts to overwrite any already bound var
//     //sequence overwritten vars must be declared as an argument to makeshot, so that python parsing can account for them
//     //It's not possible to bindVars after addEvent, since in general the added events can depend on the initial bound values

//     bool bindTargets(const std::map<std::string, RawEventTarget>& targetReplacements);
//     bool bindDeviceTargets(const std::map<std::string, RawEventTargetDevice>& targetDeviceReplacements);

//     double getTimeOffset() const;
//     void shiftStartTimeTo(double time); //shifts all events so that group start is at time
//     void shiftEndTimeTo(double time); //shifts all events so that group end is at time
//     void shiftReferenceTimeTo(const std::string& name, double time);   //shifts all events so that this reference point is at time

//     void addReferencePoint(const std::string& name, double time);   //adds a new named reference point
//     bool getReferencePoint(const std::string& name, double& time);

//     std::shared_ptr<RawEventGroup> group(const std::string& groupName);
//     std::vector<std::shared_ptr<RawEventGroup>> getSubgroups() const;

//     std::shared_ptr<RawEventVector> getEvents() const;    //should these reflect timeOrigin? no
//     std::vector<STI::Python::ParsedVar> getVars() const;
//     std::vector<STI::Python::ParsedTag> getTags() const;
    

//     // void add(const std::shared_ptr<RawEventGroup>& g, devs, vars);  //makes new group (deep copy) with new vars, except does not deep copy events

// private:

//     double timeOffset;   //tracks all time shifts
//     std::shared_ptr<RawEventVector> events;

//     std::shared_ptr<StackTraceData> stackTraceData; //global for shot
    
//     //specialize targets and vars in each group instance (deep copy)
//     std::map<std::string, RawEventTarget> targets;   //replace abstract targets in event list
//     std::map<std::string, RawEventTargetDevice> targetDevices;   //replace abstract devices in event list
//     std::vector<ParsedVar> overwrittenVars;

//     std::vector<ParsedVar> parsedVars;
//     std::vector<ParsedTag> parsedTags; 

//     // std::vector<ParsedVar> vars;
//     std::vector<std::shared_ptr<RawEventGroup>> subgroups;
//     // std::vector<std::shared_ptr<RawEventGroup>> waveforms;  //events can point to the same waveform multiple times, and any vars can be bound differently each time.
//     /*
//     waveforms unneeded? Just use a special top group name like /<waveforms>/sinepulse
//     */

//     std::string name;   //name of group relative to parent
// };




// class ShotPy : public RawEventGroup
// {
//     void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
//                 const STI::Engine::RawStackTrace& stackTrace);
//     void meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
//                 const STI::Engine::RawStackTrace& stackTrace);

//     void setvar(const std::string& name, const pybind11::object& value, 
//                 const STI::Engine::RawStackTrace& stackTrace);

//     void settag(const std::string& name, const STI::Engine::RawStackTrace& stackTrace);

//     // std::vector<std::shared_ptr<RawEventWaveform>> waveforms;
// };

//g2 = g.duplicate(devs2, vars2)
//g2.setStartTime(12*ms)

//g3.attach(g).attach(g2)
//g3.add(g).add(g2)

// class RawEventWaveform : public RawEventGroup
// {

// };

// class ShotCallback
// {
//     void getEvents();
//     void getWaveforms();
// };

// class ParsedFiles

// class Shot
// {
//     ShotConfig shotConfig;

//     std::shared_ptr<RawEventWaveform> baseGroup;    //events
    

//     std::vector<std::shared_ptr<STI::Utils::FileHolder>> timingFiles;
//     // std::vector<std::string> timingFileNames;
//     std::vector<std::string> functionNames;

//     //TEngineParsingMessageSeq messages;
//     //TEventEngineDependencyTree dependencies;

// };



class STIPyShot
{
public:

    STIPyShot(const std::shared_ptr<STI::Engine::Shot>& shot);

    // STIPyShot(const std::shared_ptr<STI::Engine::LocalShot>& shot, const std::string& name);

    void setvar(const std::string& name, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace);

    void setvar(const std::string& name, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);
    
    STI::Engine::ParsedVar var(const std::string& fullVarName, const STI::Engine::RawStackTrace& stackTrace);

    // pybind11::object getvar(const std::string& name);
    // pybind11::object getvar(const std::string& name, const STI::Engine::RawEventGroup& scope);

    void settag(const std::string& name, const STI::Engine::RawStackTrace& stackTrace);
    void settag(const std::string& name, const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);

// const STI::Engine::RawEventGroup& group
    
    void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace);
    void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);
    void meas(const STI::Engine::RawEventTarget& target, double time, 
                const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);
    void meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);

//    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts);

    std::shared_ptr<std::vector<STI::Engine::RawEvent>> getEvents();
    std::vector<STI::Engine::ParsedVar> getVars();

    // std::vector<STI::Engine::RawEvent> getEvents(const STI::Engine::RawEventGroup& group);
    // std::vector<STI::Engine::RawEvent> getEvents(const std::string& groupName);

    // std::vector<STI::Engine::ParsedVar> getVars(const STI::Engine::RawEventGroup& group);
    // std::vector<STI::Engine::ParsedVar> getVars(const std::string& groupName);

    //Not sure we need these; can be done using server->parse()
    // ParseTicket parse();
    // ParseTicket parse(const pybind11::dict& channels);
    void append(const pybind11::object& func);     //treat current list of setvars as overwritten vars
    void append(const STI::Engine::RawEvent& evt);

    // const STI::Device::DeviceID& getServerID() { return serverID; }

    std::shared_ptr<STI::Engine::Shot> getShot() { return shot; }

    std::shared_ptr<STI::Engine::RawEventGroup> group();
    std::shared_ptr<STI::Engine::RawEventGroup> group(const std::string& fullName);   //gets or makes if needed
    // std::shared_ptr<STI::Engine::RawEventGroup> group(const std::string& name, const STI::Engine::RawEventGroup& parentGroup);


private:

    // void addStackTrace(const StackTracePy& pyStackTrace, STI::Engine::StackTrace& stackTrace);
    // // void addFile(const std::string& filename);
    // unsigned addFile(const std::string& filename);


    // void addEvent(const STI::Engine::RawEventTarget& target, double time, const MixedValuePy& valuepy, 
    //                 const STI::Engine::RawEventType& type, const StackTracePy& stackTrace, 
    //                 const STI::Engine::RawEventGroup& group);

    // void addEventPy(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
    //                 const STI::Engine::RawEventType& type, const StackTracePy& stackTrace,
    //                 const STI::Engine::RawEventGroup& group);

    // std::string getAbsoluteGroupName(const std::string& groupName);
    // std::string getScopedName(const std::string& name, const STI::Engine::RawEventGroup& scope);

    // STI::Engine::RawEventGroup baseGroup;
    // unsigned nextGroupIndex;

    std::shared_ptr<STI::Engine::RawEventGroup> baseEventGroup;
    std::shared_ptr<STI::Engine::Shot> shot;
    // std::shared_ptr<STI::Engine::ParseResult> parseResult;

    // typedef STI::Utils::VectorMap<std::string, std::string> VectorMapString;
    // std::shared_ptr<VectorMapString> functionMap;

    // typedef STI::Utils::VectorMap<std::string, std::shared_ptr<STI::Utils::FileHolder>> VectorMapFileHolder;
    // std::shared_ptr<VectorMapFileHolder> fileMap;
    
    // // typedef STI::Utils::VectorMap<std::string, STI::Engine::RawEventGroup> VectorMapRawEventGroup;
    // std::shared_ptr<STI::Engine::RawEventGroupManager> groupMap;
    // // std::shared_ptr<STI::Utils::VectorMap<std::string, std::string>> groupMap;

    // typedef STI::Utils::VectorMap<std::string, STI::Engine::ParsedVar> VectorMapParsedVar;
    // std::shared_ptr<VectorMapParsedVar> varMap;

    // typedef STI::Utils::VectorMap<std::string, STI::Engine::ParsedTag> VectorMapParsedTag;
    // std::shared_ptr<VectorMapParsedTag> tagMap;


    // mutable std::mutex eventMutex;
    // unsigned eventNumber;
    // std::shared_ptr<std::vector<STI::Engine::RawEvent>> events;

    // std::shared_ptr<std::map<std::string, pybind11::object>> vars;

};


} //Python
} //STI

#endif

