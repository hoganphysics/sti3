#ifndef STI_TNETWORK_TBINARYDATASTREAM_I_H
#define STI_TNETWORK_TBINARYDATASTREAM_I_H

#include "generated/deviceNet.h"

#include <sti/utils/BinaryDataStream.h>


namespace STI
{
namespace TNetwork
{

class TBinaryDataStream_i : public POA_STI::TNetwork::TBinaryDataStream
{
public:

	TBinaryDataStream_i(STI::Utils::BinaryDataStream* dataStream);
	~TBinaryDataStream_i();
	
	void transfer(::STI::TNetwork::TBinaryDataStreamTarget_ptr target);

private:

    STI::Utils::BinaryDataStream* localBinaryDataStream;
};


} //TNetwork
} //STI


#endif

