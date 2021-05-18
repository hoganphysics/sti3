#ifndef STI_NETWORK_TSHOTREFINTERFACE_H
#define STI_NETWORK_TSHOTREFINTERFACE_H

#include "Shot.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace Network
{


//Abstract interface for extracting TShot references from appropriate Shot pointers
class TShotRefInterface
{
public:

	static bool getTShotReference(const typename std::shared_ptr<STI::Engine::Shot>& shot, STI::TNetwork::TShot_ptr& tShot)
	{
		std::shared_ptr<TShotRefInterface> tParsedShotRefInterface;
		tParsedShotRefInterface = std::dynamic_pointer_cast<TShotRefInterface>(shot);

		return (tParsedShotRefInterface != 0 &&					    //check dynamic_pointer_cast
			tParsedShotRefInterface->getTShotRef(tShot) &&	//polymorphic call
			!CORBA::is_nil(tShot)
			);
	}

private:

	virtual bool getTShotRef(STI::TNetwork::TShot_ptr& tParsedShot) = 0;

};


} //Network
} //STI


#endif

