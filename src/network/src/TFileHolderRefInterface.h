#ifndef STI_NETWORK_TFILEHOLDERREFINTERFACE_H
#define STI_NETWORK_TFILEHOLDERREFINTERFACE_H

#include <sti/utils/FileHolder.h>
#include "generated/deviceNet.h"

#include <memory>


namespace STI
{
namespace Network
{


//Abstract interface for extracting TFileHolder references from appropriate FileHolder pointers
class TFileHolderRefInterface
{
public:

	static bool getTFileHolderReference(const typename std::shared_ptr<STI::Utils::FileHolder>& fileHolder, 
                            STI::TNetwork::TFileHolder_var& tFileHolder)
	{
		std::shared_ptr<TFileHolderRefInterface> tFileHolderRefInterface;
		tFileHolderRefInterface = std::dynamic_pointer_cast<TFileHolderRefInterface>(fileHolder);

		return (tFileHolderRefInterface != 0 &&					//check dynamic_pointer_cast
			tFileHolderRefInterface->getTFileHolderRef(tFileHolder) &&	//polymorphic call
			!CORBA::is_nil(tFileHolder)
			);
	}

private:

    virtual bool getTFileHolderRef(STI::TNetwork::TFileHolder_var& tFileHolder) = 0;
};


} //Network
} //STI

#endif
