#ifndef STI_TNETWORK_TESTNETWORKWRAPPER_H
#define STI_TNETWORK_TESTNETWORKWRAPPER_H

#include "generated/deviceNet.h"
#include <memory>

#include "ServantHolder.h"

namespace STI
{
namespace TNetwork
{

class TTestNetwork_i;


class TestNetworkWrapper
{
public:

	TestNetworkWrapper(bool activate, bool deactivateOnDestruct);
    virtual ~TestNetworkWrapper();

    void getTestNetworkReference(::STI::TNetwork::TTestNetwork_var& testRef);

    // std::shared_ptr<TTestNetwork_i> test;

    ServantHolder<TTestNetwork_i, STI::TNetwork::TTestNetwork> servantHolder;

};

} //TNetwork
} //STI

#endif
