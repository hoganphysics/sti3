



#ifndef STI_PYTHON_RESULTSTICKET_H
#define STI_PYTHON_RESULTSTICKET_H


#include "fwd/Measurement_fwd.h"
#include "fwd/DeviceID_fwd.h"

namespace STI
{
namespace Python
{


class ResultTicket
{
public:

    ResultTicket();

    ResultTicket& wait();    //blocks until play completes; returns this
    void cancel();   //cancels play and stops wait()

    STI::Engine::MeasurementVector measurements();
    STI::Engine::MeasurementVector measurements(const STI::Device::DeviceID& id);

private:

};


} //Python
} //STI

#endif

