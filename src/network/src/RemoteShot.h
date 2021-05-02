#ifndef STI_NETWORK_REMOTESHOT_H
#define STI_NETWORK_REMOTESHOT_H

#include "deviceNet.h"

#include "Shot.h"

#include <memory>
#include <vector>

namespace STI
{
namespace Network
{

class RemoteShot : public STI::Engine::Shot
{
public:

	RemoteShot(::STI::TNetwork::TShot_ptr shot);
    ~RemoteShot();

    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events);

private:

    ::STI::TNetwork::TShot_var _tShot;    //remote reference
};


} //Network
} //STI


#endif

