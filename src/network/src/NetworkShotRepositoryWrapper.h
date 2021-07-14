#ifndef STI_NETWORK_NETWORKSHOTREPOSITORYWRAPPER_H
#define STI_NETWORK_NETWORKSHOTREPOSITORYWRAPPER_H

#include "ShotRepository.h"
#include "TShotRepository_i.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace Network
{


//thin wrapper around LocalDeviceEventHandler that also holds the TDeviceEventHandler_i servant of the same Handler
class NetworkShotRepositoryWrapper : public STI::Engine::ShotRepository
{
public:

	NetworkShotRepositoryWrapper(const std::shared_ptr<STI::Engine::ShotRepository>& localRepo);
	~NetworkShotRepositoryWrapper();

    bool findShot(const STI::Engine::ShotID& sid);

    bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements);
    bool getParseTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ParseTicket>& parseTicket);


	static bool getTShotRepositoryReference(
		const typename std::shared_ptr<STI::Engine::ShotRepository>& shotRepo,
		STI::TNetwork::TShotRepository_var& tShotRepo)
	{
		auto wrapper = std::dynamic_pointer_cast<NetworkShotRepositoryWrapper>(shotRepo);
		if (wrapper) {
			tShotRepo = wrapper->shotRepositoryServant._this();
			return !CORBA::is_nil(tShotRepo);
		}
		return false;
	}

	void disable();

private:

	std::shared_ptr<STI::Engine::ShotRepository> localRepository;

	STI::TNetwork::TShotRepository_i shotRepositoryServant;

};

} //Network
} //STI


#endif

