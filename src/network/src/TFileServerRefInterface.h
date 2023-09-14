#ifndef STI_NETWORK_TFILESERVERREFINTERFACE_H
#define STI_NETWORK_TFILESERVERREFINTERFACE_H

#include <sti/utils/FileServer.h>
#include "generated/deviceNet.h"

#include <memory>


namespace STI
{
namespace Network
{


//Abstract interface for extracting TFileServer references from appropriate FileServer pointers
class TFileServerRefInterface
{
public:

	static bool getTFileServerReference(const typename std::shared_ptr<STI::Utils::FileServer>& fileServer, 
                            STI::TNetwork::TFileServer_var& tFileServer)
	{
		std::shared_ptr<TFileServerRefInterface> tFileServerRefInterface;
		tFileServerRefInterface = std::dynamic_pointer_cast<TFileServerRefInterface>(fileServer);

		return (tFileServerRefInterface != 0 &&					//check dynamic_pointer_cast
			tFileServerRefInterface->getTFileServerRef(tFileServer) &&	//polymorphic call
			!CORBA::is_nil(tFileServer)
			);
	}

private:

    virtual bool getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer) = 0;
};


} //Network
} //STI

#endif
