
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

    // std::shared_ptr<STI::Engine::ShotResult> getShot(const STI::Engine::ShotID& sid);
    std::map<STI::Device::DeviceID, STI::Engine::MeasurementVector> getMeasurements(const STI::Engine::ShotID& sid);

    bool findShot(const STI::Engine::ShotID& sid);
    std::shared_ptr<STI::Engine::ParseResult> getParseResult(const STI::Engine::ParseID& pid);
    std::shared_ptr<STI::Engine::ShotResult> getShotResult(const STI::Engine::ShotID& sid);
    std::shared_ptr<STI::Engine::SequenceResult> getSequenceResult(const STI::Engine::SequenceID& id);


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

