
#ifndef STI_PYTHON_STIPYSHOT_H
#define STI_PYTHON_STIPYSHOT_H

#include <sti/fwd/RawEvent_fwd.h>

#include "Shot.h"
#include <sti/device/DeviceID.h>
#include <sti/engine/StackTrace.h>
#include "RawEventGroup.h"
#include <sti/engine/RawEventTarget.h>
#include "RawEventGroupManager.h"

#include <sti/utils/VectorMap.h>

#include "StackTracePy.h"

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
class ParsedVarPy;

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

*/

class STIPyShot
{
public:

    STIPyShot(const std::shared_ptr<STI::Engine::Shot>& shot, const std::string& name);

    void setvar(const std::string& name, const pybind11::object& value, 
                const StackTracePy& stackTrace);

    void setvar(const std::string& name, const pybind11::object& value, 
                const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& scope);
    
    pybind11::object getvar(const std::string& name);
    pybind11::object getvar(const std::string& name, const STI::Engine::RawEventGroup& scope);

    void settag(const std::string& name, const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& scope);

// const STI::Engine::RawEventGroup& group

    void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group);
    void meas(const STI::Engine::RawEventTarget& target, double time, 
                const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group);
    void meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group);

//    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts);

    std::vector<STI::Engine::RawEvent> getEvents();

    std::vector<STI::Python::ParsedVarPy> getVars();

    //Not sure we need these; can be done using server->parse()
    // ParseTicket parse();
    // ParseTicket parse(const pybind11::dict& channels);
    void append(pybind11::object func);     //treat current list of setvars as overwritten vars
    void append(const STI::Engine::RawEvent& evt);

    // const STI::Device::DeviceID& getServerID() { return serverID; }

    std::shared_ptr<STI::Engine::Shot> getShot() { return shot; }

    STI::Engine::RawEventGroup group(const std::string& fullName);   //gets or makes if needed
    STI::Engine::RawEventGroup group(const std::string& name, const STI::Engine::RawEventGroup& parentGroup);


private:

    void addStackTrace(const StackTracePy& pyStackTrace, STI::Engine::StackTrace& stackTrace);
    // void addFile(const std::string& filename);
    unsigned addFile(const std::string& filename);


    void addEvent(const STI::Engine::RawEventTarget& target, double time, const MixedValuePy& valuepy, 
                    const STI::Engine::RawEventType& type, const StackTracePy& stackTrace, 
                    const STI::Engine::RawEventGroup& group);

    void addEventPy(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                    const STI::Engine::RawEventType& type, const StackTracePy& stackTrace,
                    const STI::Engine::RawEventGroup& group);

    std::string getAbsoluteGroupName(const std::string& groupName);
    std::string getScopedName(const std::string& name, const STI::Engine::RawEventGroup& scope);

    STI::Engine::RawEventGroup baseGroup;
    unsigned nextGroupIndex;

    std::shared_ptr<STI::Engine::Shot> shot;
    std::shared_ptr<STI::Engine::ParseResult> parseResult;

    typedef STI::Utils::VectorMap<std::string, std::string> VectorMapString;
    std::shared_ptr<VectorMapString> functionMap;

    typedef STI::Utils::VectorMap<std::string, std::shared_ptr<STI::Utils::FileHolder>> VectorMapFileHolder;
    std::shared_ptr<VectorMapFileHolder> fileMap;
    
    // typedef STI::Utils::VectorMap<std::string, STI::Engine::RawEventGroup> VectorMapRawEventGroup;
    std::shared_ptr<STI::Engine::RawEventGroupManager> groupMap;
    // std::shared_ptr<STI::Utils::VectorMap<std::string, std::string>> groupMap;

    typedef STI::Utils::VectorMap<std::string, STI::Engine::ParsedVar> VectorMapParsedVar;
    std::shared_ptr<VectorMapParsedVar> varMap;

    typedef STI::Utils::VectorMap<std::string, STI::Engine::ParsedTag> VectorMapParsedTag;
    std::shared_ptr<VectorMapParsedTag> tagMap;


    mutable std::mutex eventMutex;
    unsigned eventNumber;
    // std::shared_ptr<std::vector<STI::Engine::RawEvent>> events;

    // std::shared_ptr<std::map<std::string, pybind11::object>> vars;

};


} //Python
} //STI

#endif

