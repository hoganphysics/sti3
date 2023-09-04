#ifndef STI_TNETWORK_TSHOTCALLBACK_I_H
#define STI_TNETWORK_TSHOTCALLBACK_I_H

#include <sti/engine/Shot.h>
#include "generated/deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TShotCallback_i : public POA_STI::TNetwork::TShotCallback
{
public:

	TShotCallback_i(const std::shared_ptr<STI::Engine::Shot>& shot);
	~TShotCallback_i();

    void getRootEventGroup(::STI::TNetwork::TRawEventGroup_out rootGroup);

private:

	std::shared_ptr<STI::Engine::Shot> localShot;
};

} //TNetwork
} //STI

#endif
