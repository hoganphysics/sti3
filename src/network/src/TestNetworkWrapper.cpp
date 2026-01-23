#include "TestNetworkWrapper.h"

#include "TTestNetwork_i.h"
#include "ORBManager.h"

using STI::TNetwork::TestNetworkWrapper;

TestNetworkWrapper::TestNetworkWrapper(bool activate,  bool deactivateOnDestruc)
: servantHolder(new TTestNetwork_i(deactivateOnDestruc))
{
}

TestNetworkWrapper::~TestNetworkWrapper()
{
}

void TestNetworkWrapper::getTestNetworkReference(::STI::TNetwork::TTestNetwork_var& testRef)
{
    testRef = servantHolder.getRefVar();
}
