#ifndef STI_TNETWORK_TPARSEDSHOT_I_H
#define STI_TNETWORK_TPARSEDSHOT_I_H

#include "Shot.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TParsedShot_i : public POA_STI::TNetwork::TParsedShot
{
public:

	TParsedShot_i(const std::shared_ptr<STI::Engine::Shot>& shot);
	~TParsedShot_i();

    void getEvents(::STI::TNetwork::TRawEventSeq_out events);

private:

	std::shared_ptr<STI::Engine::Shot> localShot;
};

} //TNetwork
} //STI

#endif
