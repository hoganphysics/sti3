#ifndef STI_TNETWORK_TDEVICEHUB_I_H
#define STI_TNETWORK_TDEVICEHUB_I_H

#include "generated/deviceNet.h"

#include <sti/device/Device.h>
#include <sti/LocalDeviceHub.h>

#include <memory>


namespace STI
{
namespace TNetwork
{


class TDeviceHub_i : public POA_STI::TNetwork::TDeviceHub, 
					 public PortableServer::RefCountServantBase
{
public:

	TDeviceHub_i(const std::shared_ptr<STI::Network::LocalDeviceHub>& hub);
	~TDeviceHub_i();

	::CORBA::Boolean addHub(const ::STI::TNetwork::TDeviceHubID& hubID, ::STI::TNetwork::TDeviceHub_ptr hub);
	::CORBA::Boolean removeHub(const ::STI::TNetwork::TDeviceHubID& hubID);
	::CORBA::Boolean refreshNetwork(const ::STI::TNetwork::TDeviceHubTrace& trace);
	::CORBA::Boolean distribute(const ::STI::TNetwork::TDeviceID& devID, ::STI::TNetwork::TDevice_ptr node, const ::STI::TNetwork::TDeviceHubTrace& trace, const ::STI::TNetwork::TDeviceHubID& first);
	::CORBA::Boolean distributeNodes(const ::STI::TNetwork::TDeviceHubID& targetHub);
	::CORBA::Boolean redistributeNodes(const ::STI::TNetwork::TDeviceHubTrace& trace);
	::CORBA::Boolean removeNode(const ::STI::TNetwork::TDeviceID& devID, const ::STI::TNetwork::TDeviceHubTrace& trace);
	TDeviceHubID* deviceHubID();
	::CORBA::Boolean hasNodeID(const ::STI::TNetwork::TDeviceID& devID);
	void walk(::STI::TNetwork::TNodeWalker& root, const ::STI::TNetwork::TDeviceHubTrace& trace);

private:

	std::shared_ptr<STI::Network::LocalDeviceHub> localHub;
};

} //TNetwork
} //STI


#endif

