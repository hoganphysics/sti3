#include "TTestNetwork_i.h"
// #include "ORBManager.h"

#include <iostream>

using STI::TNetwork::TTestNetwork_i;


TTestNetwork_i::TTestNetwork_i(bool deactivateTest)
    : deactivateTest(deactivateTest)
{
    std::cout << "TTestNetwork_i constructor called." << std::endl;
}

TTestNetwork_i::~TTestNetwork_i()
{
    std::cout << "TTestNetwork_i destructor called." << std::endl;

    if (deactivateTest) {
        // STI::Network::ORBManager::ORBManager::deactivateServant(this, true);
    }
}


::CORBA::Boolean TTestNetwork_i::ping()
{
    return true;
}

