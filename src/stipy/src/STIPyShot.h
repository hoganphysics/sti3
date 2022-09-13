#ifndef STI_PYTHON_STIPYSHOT_H
#define STI_PYTHON_STIPYSHOT_H

#include <sti/fwd/RawEvent_fwd.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/StackTrace.h>
#include <sti/utils/VectorMap.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileHolderFactory.h>

#include "LocalShot.h"
#include <sti/engine/RawEventGroup.h>
#include "RawStackTrace.h"

#include <vector>
#include <memory>
#include <mutex>
#include <map>

#include <pybind11/pybind11.h>

namespace STI
{
namespace Python
{

class STIPyServer;
class ParseTicket;
class MixedValuePy;


class STIPyShot
{
public:

    STIPyShot(const std::shared_ptr<STI::Engine::Shot>& shot);

    void setvar(const std::string& name, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace);

    void setvar(const std::string& name, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);
    
    STI::Engine::ParsedVar var(const std::string& fullVarName, const STI::Engine::RawStackTrace& stackTrace);

    void settag(const std::string& name, const STI::Engine::RawStackTrace& stackTrace);
    void settag(const std::string& name, const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);
  
    void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace);
    void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);
    void meas(const STI::Engine::RawEventTarget& target, double time, 
                const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);
    void meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace, const std::string& scope);

    std::shared_ptr<std::vector<STI::Engine::RawEvent>> getEvents();
    std::vector<STI::Engine::ParsedVar> getVars();

    void append(const pybind11::object& func);     //treat current list of setvars as overwritten vars
    void append(const STI::Engine::RawEvent& evt);

    std::shared_ptr<STI::Engine::Shot> getShot() { return shot; }

    std::shared_ptr<STI::Engine::RawEventGroup> group();
    std::shared_ptr<STI::Engine::RawEventGroup> group(const std::string& fullName);   //gets or makes if needed

private:

    std::shared_ptr<STI::Engine::RawEventGroup> rootEventGroup;
    std::shared_ptr<STI::Engine::Shot> shot;

};


} //Python
} //STI

#endif

