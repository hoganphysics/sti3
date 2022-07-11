
#ifndef STI_PYTHON_STIPYGLOBAL_H
#define STI_PYTHON_STIPYGLOBAL_H

#include <sti/engine/StackTrace.h>
#include "RawEventGroup.h"
#include <sti/engine/RawEventTarget.h>

#include <functional>
#include <memory>
#include <mutex>

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
    void makeShot(const std::shared_ptr<STIPyShot>& shot, const std::string& name, const std::function<void(void)>& func);

    // void event(const RawEventTarget& channel, double time, const pybind11::object& value, 
    //             const STI::Engine::StackTrace& stackTrace, const STI::Engine::RawEventGroup& group);

    void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value,
                const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group);
    void meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value,
                const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group);
    void meas(const STI::Engine::RawEventTarget& target, double time, const StackTracePy& stackTrace, 
                const STI::Engine::RawEventGroup& group);

    // STI::Engine::RawEventTargetDevice dev(const std::string& name, const std::string& address, unsigned module);


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

