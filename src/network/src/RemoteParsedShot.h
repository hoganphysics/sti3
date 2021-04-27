#ifndef STI_NETWORK_REMOTEPARSEDSHOT_H
#define STI_NETWORK_REMOTEPARSEDSHOT_H

#include "deviceNet.h"

#include "Shot.h"

#include <memory>
#include <vector>

namespace STI
{
namespace Network
{

class RemoteParsedShot : public STI::Engine::Shot
{
public:

	RemoteParsedShot(::STI::TNetwork::TParsedShot_ptr shot);
    ~RemoteParsedShot();

    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events);

private:

    ::STI::TNetwork::TParsedShot_var _tShot;    //remote reference
};


} //Network
} //STI


#endif

