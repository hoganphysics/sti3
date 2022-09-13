#ifndef STI_PYTHON_STIPYCHANNEL_H
#define STI_PYTHON_STIPYCHANNEL_H

#include "STIPyDevice.h"

#include <string>
#include <memory>

namespace STI
{
namespace Python
{

class STIPyDevice;


class STIPyChannel
{
public:

    STIPyChannel(const std::string& name);  //abstract
    STIPyChannel(const std::shared_ptr<STIPyDevice>& device, const std::string& name);   //abstract

    STIPyChannel(const std::shared_ptr<STIPyDevice>& device, unsigned channel);

    bool isAbstract() const;
    std::string abstractName() const;

    std::shared_ptr<STIPyDevice> device() const;
    unsigned channel() const;

    std::string print() const;

private:

    std::shared_ptr<STIPyDevice> device_;
    unsigned channel_;

    bool abstract_;
    bool hasDevice_;
    std::string abstractName_;

};


} //Python
} //STI

#endif

