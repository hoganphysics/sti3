#ifndef STI_ENGINE_NETWORKPARSEDSHOTWRAPPER_H
#define STI_ENGINE_NETWORKPARSEDSHOTWRAPPER_H

#include "ParsedShot.h"
//#include "TParsedShotRefInterface.h"

#include "TParsedShot_i.h"
#include "deviceNet.h"

#include <vector>
#include <memory>

namespace STI
{
namespace Network
{




class NetworkParsedShotWrapper : public STI::Engine::ParsedShot
                                 //public STI::Network::TParsedShotRefInterface	//mixin
{
public:


    NetworkParsedShotWrapper(const std::shared_ptr<STI::Engine::ParsedShot>& shot)
    : localshot(shot), parsedShotServant(shot)
    {
    }

    ~NetworkParsedShotWrapper()
    {
    }

    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
    {
        if (localshot != 0) {
            localshot->getEvents(evts);
        }
    }

	static bool getTParsedShotReference(
		const typename std::shared_ptr<STI::Engine::ParsedShot>& shot, 
        STI::TNetwork::TParsedShot_ptr& tShot)
	{
		auto wrapper = std::dynamic_pointer_cast<NetworkParsedShotWrapper>(shot);
		if (wrapper) {
			tShot = wrapper->parsedShotServant._this();
			return !CORBA::is_nil(tShot);
		}
		return false;
	}

private:

    // bool getTParsedShotRef(STI::TNetwork::TParsedShot_ptr& tParsedShot)
    // {
    //     STI::TNetwork::TParsedShot_var newShot;
    //     newShot = _tShot;		//implicit duplicate

    //     tParsedShot = newShot.out();
    //     return !CORBA::is_nil(tParsedShot);
    // }

    





    std::shared_ptr<STI::Engine::ParsedShot> localshot;
//    ::STI::TNetwork::TParsedShot_var _tShot;		//remote reference
    STI::TNetwork::TParsedShot_i parsedShotServant;

};


} //Network
} //STI

#endif
