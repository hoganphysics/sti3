


#ifndef STI_PYTHON_PYRESULTSTICKET_H
#define STI_PYTHON_PYRESULTSTICKET_H

#include "ResultTicket.h"


namespace STI
{
namespace Python
{


class PyResultTicket : public STI::Engine::ResultTicket
{
public:

    PyResultTicket(const STI::Engine::ShotID& id, 
                const std::shared_ptr<STI::Device::Device>& server);
    PyResultTicket(const STI::Engine::ShotID& id, 
                const std::shared_ptr<STI::Device::Device>& server, const TicketStatus& initialStatus);

private:

    bool waitCheck();

};


} //Python
} //STI

#endif

