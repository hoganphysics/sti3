#ifndef STI_TNETWORK_TBINARYDATASTREAMTARGET_I_H
#define STI_TNETWORK_TBINARYDATASTREAMTARGET_I_H

#include "generated/deviceNet.h"

#include <sti/utils/BinaryDataStream.h>


namespace STI
{
namespace TNetwork
{

class TBinaryDataStreamTarget_i : public POA_STI::TNetwork::TBinaryDataStreamTarget,
								  public PortableServer::RefCountServantBase
{
public:

	TBinaryDataStreamTarget_i(STI::Utils::BinaryDataStreamTarget* target);
	~TBinaryDataStreamTarget_i();
	
	void start();
	void writeNext(const ::STI::TNetwork::TBinaryData& data);
	void stop();

private:

    STI::Utils::BinaryDataStreamTarget* localBinaryDataStreamTarget;
};


} //TNetwork
} //STI


#endif

