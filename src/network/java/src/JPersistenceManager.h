
#ifndef STI_DEVICE_JPERSISTENCEMANAGER_H
#define STI_DEVICE_JPERSISTENCEMANAGER_H

#include "PersistenceManager.h"

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
	
	JPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& manager);
	~JPersistenceManager();

    std::shared_ptr<STI::Engine::ShotResult> getShot(const STI::Engine::ShotID& sid);
    std::shared_ptr<STI::Engine::MeasurementVector> getMeasurements(const STI::Engine::ShotID& sid);

private:

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;

};

} //Device
} //STI

#endif
