
#ifndef STI_PYTHON_PERSISTENCEMANAGER_H
#define STI_PYTHON_PERSISTENCEMANAGER_H

#include <sti/device/PersistenceManager.h>

#include <memory>



namespace STI
{
namespace Python
{

class PersistenceManagerPy
{
public:

    PersistenceManagerPy(const std::shared_ptr<STI::Device::PersistenceManager>& manager);
    ~PersistenceManagerPy();

    bool findShot(const STI::Engine::ShotID& sid);
    std::shared_ptr<STI::Engine::ParseResult> getParseResult(const STI::Engine::ParseID& pid);
    std::shared_ptr<STI::Engine::ShotResult> getShotResult(const STI::Engine::ShotID& sid);
    std::shared_ptr<STI::Engine::SequenceResult> getSequenceResult(const STI::Engine::SequenceID& id);

	STI::Engine::MeasurementMap getMeasurements(const STI::Engine::ShotID& sid);

private:

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
};


} //Python
} //STI

#endif

