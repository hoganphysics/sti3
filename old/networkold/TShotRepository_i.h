#ifndef STI_TNETWORK_TSHOTREPOSITORY_I_H
#define STI_TNETWORK_TSHOTREPOSITORY_I_H

#include <sti/engine/ShotRepository.h>
#include "deviceNet.h"

#include <memory>


namespace STI
{
namespace TNetwork
{


class TShotRepository_i : public POA_STI::TNetwork::TShotRepository
{
public:

	TShotRepository_i(const std::shared_ptr<STI::Engine::ShotRepository>& shotRepository);
	~TShotRepository_i();

    ::CORBA::Boolean findShot(const ::STI::TNetwork::TShotID& sid);

    ::CORBA::Boolean getMeasurements(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TMeasurementSeq_out measurements);
    
private:

    std::shared_ptr<STI::Engine::ShotRepository> shotRepository;

};


} //TNetwork
} //STI

#endif

