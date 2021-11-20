#ifndef STI_TNETWORK_TSHOTEVENTSCALLBACK_I_H
#define STI_TNETWORK_TSHOTEVENTSCALLBACK_I_H

#include "Shot.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TShotEventsCallback_i : public POA_STI::TNetwork::TShotEventsCallback
{
public:

	TShotEventsCallback_i(const std::shared_ptr<STI::Engine::Shot>& shot);
	~TShotEventsCallback_i();

    void getEvents(::STI::TNetwork::TRawEventSeq_out events);

private:

	std::shared_ptr<STI::Engine::Shot> localShot;
};

} //TNetwork
} //STI

#endif
