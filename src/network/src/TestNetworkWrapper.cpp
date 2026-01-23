#include "TestNetworkWrapper.h"

#include "TTestNetwork_i.h"
#include "ORBManager.h"

using STI::TNetwork::TestNetworkWrapper;

TestNetworkWrapper::TestNetworkWrapper(bool activate,  bool deactivateOnDestruc)
: servantHolder(new TTestNetwork_i(deactivateOnDestruc))
{
    // test = std::make_shared<TTestNetwork_i>(deactivateOnDestruc);

    // if (activate) {
    //     STI::Network::ORBManager::ORBManager::activateServant(*test);
    // }
    // test->_this();
    
}

TestNetworkWrapper::~TestNetworkWrapper()
{
}

void TestNetworkWrapper::getTestNetworkReference(::STI::TNetwork::TTestNetwork_var& testRef)
{
    // testRef = test->_this();
    testRef = servantHolder.getRefVar();
}
