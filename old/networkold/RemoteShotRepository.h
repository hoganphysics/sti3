
#ifndef STI_NETWORK_REMOTESHOTREPOSITORY_H
#define STI_NETWORK_REMOTESHOTREPOSITORY_H

#include "deviceNet.h"

#include <sti/engine/ShotRepository.h>
#include "TReferenceHolder.h"


#include <memory>
#include <vector>


namespace STI
{
namespace Network
{

class RemoteShotRepository : public STI::Engine::ShotRepository,
                             public STI::TNetwork::TReferenceHolder<STI::TNetwork::TShotRepository>	//mixin

{
public:

	RemoteShotRepository(::STI::TNetwork::TShotRepository_ptr repo);
    ~RemoteShotRepository();

    bool findShot(const STI::Engine::ShotID& sid);

    bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements);
    bool getParseTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ParseTicket>& parseTicket);

private:

    mutable std::mutex shotRepoMutex;
};


} //Network
} //STI


#endif

