#include "PyResultTicket.h"

#include <pybind11/pybind11.h>


using STI::Python::PyResultTicket;
using STI::Engine::ResultTicket;
using STI::Engine::ShotID;

namespace py = pybind11;


PyResultTicket::PyResultTicket(const ShotID& id, const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager)
: ResultTicket(id, persistenceManager)
{
}

PyResultTicket::PyResultTicket(const ShotID& id, 
            const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager, const TicketStatus& initialStatus)
: ResultTicket(id, persistenceManager, initialStatus)
{
}

// std::map<STI::Device::DeviceID, STI::Engine::MeasurementVector> PyResultTicket::pyMeasurements()
// {
//     std::map<STI::Device::DeviceID, STI::Engine::MeasurementVector> pyMeasurements;
//     std::shared_ptr<STI::Engine::MeasurementMap> measurements;
    
//     if (getMeasurements(measurements)) {
//         for (auto& tuple : *measurements) {
//             if (tuple.second != 0) {
//                 auto& mVec = pyMeasurements[tuple.first];
//                 mVec.insert(mVec.end(), tuple.second->begin(), tuple.second->end());
//             }
//         }
//     }
//     return pyMeasurements;
// }

bool PyResultTicket::waitCheck()
{
    if (PyErr_CheckSignals() != 0) 
        throw py::error_already_set();
    
    return true;
}

