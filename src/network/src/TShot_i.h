#ifndef STI_TNETWORK_TSHOT_I_H
#define STI_TNETWORK_TSHOT_I_H

#include "Shot.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TShot_i : public POA_STI::TNetwork::TShot
{
public:

	TShot_i(const std::shared_ptr<STI::Engine::Shot>& shot);
	~TShot_i();

    void getEvents(::STI::TNetwork::TRawEventSeq_out events);

private:

	std::shared_ptr<STI::Engine::Shot> localShot;
};

} //TNetwork
} //STI

#endif
