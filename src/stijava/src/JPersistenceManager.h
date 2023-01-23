
#ifndef STI_DEVICE_JPERSISTENCEMANAGER_H
#define STI_DEVICE_JPERSISTENCEMANAGER_H

#include <sti/device/PersistenceManager.h>

#include <memory>


namespace STI
{
namespace Device
{

class PersistenceManager;


//Java AttributeManager wrapper
class JPersistenceManager
{
public:
	
	JPersistenceManager(const std::shared_ptr<STI::Device::PersistenceManager>& manager);
	~JPersistenceManager();

    std::shared_ptr<STI::Engine::ParseResult> getParseResult(const STI::Engine::ParseID& pid);
    std::shared_ptr<STI::Engine::ShotResult> getShotResult(const STI::Engine::ShotID& sid);

    std::shared_ptr<STI::Engine::MeasurementMap> getMeasurements(const STI::Engine::ShotID& sid);

private:

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;

};

} //Device
} //STI

#endif
