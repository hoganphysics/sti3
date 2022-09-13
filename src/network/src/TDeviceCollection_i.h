#ifndef STI_TNETWORK_TDEVICECOLLECTION_I_H
#define STI_TNETWORK_TDEVICECOLLECTION_I_H

#include "deviceNet.h"

#include <sti/device/DeviceCollection.h>

#include <memory>

namespace STI
{
namespace TNetwork
{

class TDeviceCollection_i : public POA_STI::TNetwork::TDeviceCollection
{
public:

	TDeviceCollection_i(const std::shared_ptr<STI::Device::DeviceCollector>& collector);
	~TDeviceCollection_i();

	::CORBA::Boolean add(const ::STI::TNetwork::TDeviceID& deviceID, ::STI::TNetwork::TDevice_ptr device);
	::CORBA::Boolean remove(const ::STI::TNetwork::TDeviceID& deviceID);
	::CORBA::Boolean contains(const ::STI::TNetwork::TDeviceID& deviceID);
	::CORBA::ULong size();
	::CORBA::Boolean get(const ::STI::TNetwork::TDeviceID& deviceID, ::STI::TNetwork::TDevice_out device);
	void getIDs(::STI::TNetwork::TDeviceIDSeq_out deviceIDseq);
	void cleanup();
	void clear();
	::CORBA::Boolean ping();

private:

	std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;
};

} //TNetwork
} //STI


#endif

