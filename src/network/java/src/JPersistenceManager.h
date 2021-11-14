
#ifndef STI_DEVICE_JPERSISTENCEMANAGER_H
#define STI_DEVICE_JPERSISTENCEMANAGER_H

#include "AttributeManager.h"

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

    bool getResultTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ResultTicket>& ticket);
    bool getShotRepository(std::shared_ptr<STI::Engine::ShotRepository>& repo);

private:

    std::shared_ptr<STI::Device::PersistenceManager> localManager;

};

} //Device
} //STI

#endif
