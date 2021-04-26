#ifndef STI_NETWORK_TPARSEDSHOTREFINTERFACE_H
#define STI_NETWORK_TPARSEDSHOTREFINTERFACE_H

#include "Shot.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace Network
{


//Abstract interface for extracting TParsedShot references from appropriate Shot pointers
class TParsedShotRefInterface
{
public:

	static bool getTParsedShotReference(const typename std::shared_ptr<STI::Engine::Shot>& shot, STI::TNetwork::TParsedShot_ptr& tShot)
	{
		std::shared_ptr<TParsedShotRefInterface> tParsedShotRefInterface;
		tParsedShotRefInterface = std::dynamic_pointer_cast<TParsedShotRefInterface>(shot);

		return (tParsedShotRefInterface != 0 &&					    //check dynamic_pointer_cast
			tParsedShotRefInterface->getTParsedShoteRef(tShot) &&	//polymorphic call
			!CORBA::is_nil(tShot)
			);
	}

private:

	virtual bool getTParsedShoteRef(STI::TNetwork::TParsedShot_ptr& tParsedShot) = 0;

};


} //Network
} //STI


#endif

