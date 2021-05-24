
#ifndef STI_PYTHON_STIPYGLOBAL_H
#define STI_PYTHON_STIPYGLOBAL_H


#include <functional>
#include <memory>
#include <mutex>

#include <pybind11/pybind11.h>


namespace STI
{
namespace Python
{


class STIPyShot;
class STIPyDevice;
class STIPyChannel;
class STIPyGlobal;
struct Concrete_STIPyGlobal;


class STIPyGlobal
{
public:

    virtual ~STIPyGlobal();

    static std::shared_ptr<STIPyGlobal> getInstance();

    void makeShot(const std::shared_ptr<STIPyShot>& shot, const std::function<void(void)>& func);

    void event(const STIPyChannel& channel, double time, const pybind11::object& value);
    void meas(const STIPyChannel& channel, double time, const pybind11::object& value);

    std::shared_ptr<STIPyDevice> dev(const std::string& name, const std::string& address, unsigned module);

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

