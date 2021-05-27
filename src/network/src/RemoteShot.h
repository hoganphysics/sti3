#ifndef STI_NETWORK_REMOTESHOT_H
#define STI_NETWORK_REMOTESHOT_H

#include "deviceNet.h"

#include "Shot.h"
#include "TReferenceHolder.h"

#include <memory>
#include <vector>

namespace STI
{
namespace Network
{

class RemoteShot : public STI::Engine::Shot,
                   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TShot>	//mixin
{
public:

	RemoteShot(::STI::TNetwork::TShot_ptr shot);
    ~RemoteShot();

    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events);

private:

    //::STI::TNetwork::TShot_var _tShot;    //remote reference
    mutable std::mutex shotMutex;
};


} //Network
} //STI


#endif

