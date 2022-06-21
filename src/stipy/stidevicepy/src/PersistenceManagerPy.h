
#ifndef STI_PYTHON_PERSISTENCEMANAGER_H
#define STI_PYTHON_PERSISTENCEMANAGER_H

#include "PersistenceManager.h"

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

    std::shared_ptr<STI::Engine::ShotResult> getShot(const STI::Engine::ShotID& sid);
    STI::Engine::MeasurementVector getMeasurements(const STI::Engine::ShotID& sid);

private:
    // bool getShot(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result);
    // bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::ShotResult>& shotResult, bool isOwner);
    // STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector);
    // bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements);
	// void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
};


} //Python
} //STI

#endif

