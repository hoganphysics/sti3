#ifndef STI_TNETWORK_TTESTNETWORK_I_H
#define STI_TNETWORK_TTESTNETWORK_I_H

#include "generated/deviceNet.h"
#include "TriggerCallback.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TTestNetwork_i : public POA_STI::TNetwork::TTestNetwork, 
                       public PortableServer::RefCountServantBase 
{
public:

	TTestNetwork_i(bool deactivateTest);
    virtual ~TTestNetwork_i();

    ::CORBA::Boolean ping();

private:

    bool deactivateTest;

};

} //TNetwork
} //STI

#endif
