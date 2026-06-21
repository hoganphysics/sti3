
#ifndef STI_PYTHON_STIPYGLOBAL_H
#define STI_PYTHON_STIPYGLOBAL_H

#include <sti/engine/CompressedStackTrace.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/PostProcessTarget.h>
#include <sti/engine/ParsedVar.h>

#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include <pybind11/pybind11.h>


namespace STI
{
namespace Python
{


class STIPyShot;
class STIPyGlobal;
struct Concrete_STIPyGlobal;
class StackTracePy;


class STIPyGlobal
{
public:

    virtual ~STIPyGlobal();

    static std::shared_ptr<STIPyGlobal> getInstance();

    void makeShot(const std::shared_ptr<STIPyShot>& shot, const std::function<void(void)>& func);
    void makeShot(const std::shared_ptr<STIPyShot>& shot, const std::function<void(void)>& func, const std::string& mainFile);

    STI::Engine::ParsedVar var(const std::string& fullVarName, const STI::Engine::StackTrace& stackTrace);

    void setvar(const std::string& name, const pybind11::object& value, 
                const STI::Engine::StackTrace& stackTrace, const std::string& scope);
    void settag(const std::string& name, const STI::Engine::StackTrace& stackTrace, const std::string& scope);

    void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value,
                const STI::Engine::StackTrace& stackTrace);
    void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value,
                const STI::Engine::StackTrace& stackTrace, const std::string& scope);
    void meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value,
                const STI::Engine::StackTrace& stackTrace, const std::string& scope);
    void meas(const STI::Engine::RawEventTarget& target, double time, 
                const STI::Engine::StackTrace& stackTrace, const std::string& scope);

    void set_trigger(const STI::Device::DeviceID& deviceID, const STI::Engine::StackTrace& stackTrace);

    void postProcess(const STI::Engine::PostProcessTarget& target, const pybind11::object& options,
                const STI::Engine::StackTrace& stackTrace);

    std::shared_ptr<STI::Engine::RawEventGroup> group(const std::string& name);

private:

    STIPyGlobal();

    friend Concrete_STIPyGlobal;

    static std::shared_ptr<STIPyGlobal> instance;
    static bool initialized;

    bool makingShot;
    std::shared_ptr<STIPyShot> currentShot;

    mutable std::mutex shotMutex;

};


struct Concrete_STIPyGlobal : public STIPyGlobal 
{
};


} //Python
} //STI

#endif
