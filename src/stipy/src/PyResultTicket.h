#ifndef STI_PYTHON_PYRESULTSTICKET_H
#define STI_PYTHON_PYRESULTSTICKET_H

#include <sti/engine/ResultTicket.h>


namespace STI
{
namespace Python
{


class PyResultTicket : public STI::Engine::ResultTicket
{
public:

    PyResultTicket(const STI::Engine::ShotID& id, 
                const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager);
    PyResultTicket(const STI::Engine::ShotID& id, 
                const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager, const TicketStatus& initialStatus);

    // std::map<STI::Device::DeviceID, STI::Engine::MeasurementVector> pyMeasurements();

private:

    bool waitCheck() const;

};


} //Python
} //STI

#endif

