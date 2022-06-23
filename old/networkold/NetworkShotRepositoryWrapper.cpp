
#include "NetworkShotRepositoryWrapper.h"

using STI::Network::NetworkShotRepositoryWrapper;


NetworkShotRepositoryWrapper::NetworkShotRepositoryWrapper(const std::shared_ptr<STI::Engine::ShotRepository>& localRepo)
: shotRepositoryServant(localRepo), localRepository(localRepo)
{
}

NetworkShotRepositoryWrapper::~NetworkShotRepositoryWrapper()
{
}

bool NetworkShotRepositoryWrapper::findShot(const STI::Engine::ShotID& sid)
{
    if (localRepository != 0) {
		return localRepository->findShot(sid);
	}
	return false;
}

bool NetworkShotRepositoryWrapper::getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements)
{
	if (localRepository != 0) {
		return localRepository->getMeasurements(sid, measurements);
	}
	return false;
}

bool NetworkShotRepositoryWrapper::getParseTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ParseTicket>& parseTicket)
{
	if (localRepository != 0) {
		return localRepository->getParseTicket(sid, parseTicket);
	}
	return false;
}
