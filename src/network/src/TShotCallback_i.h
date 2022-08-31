#ifndef STI_TNETWORK_TSHOTCALLBACK_I_H
#define STI_TNETWORK_TSHOTCALLBACK_I_H

#include "Shot.h"
#include "deviceNet.h"

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

    // void getGroups(::STI::TNetwork::TRawEventGroup_out groups);
    // void getVars(::STI::TNetwork::TParsedVarSeq_out parsedVars);
    // void getTags(::STI::TNetwork::TParsedTagSeq_out parsedTags);
    // void getParseResult(::STI::TNetwork::TParseResult_out parseResult);
    // void getEvents(::STI::TNetwork::TRawEventSeq_out events);

    void getBaseEventGroup(::STI::TNetwork::TRawEventGroup_out baseGroup);

private:

	std::shared_ptr<STI::Engine::Shot> localShot;
};

} //TNetwork
} //STI

#endif
