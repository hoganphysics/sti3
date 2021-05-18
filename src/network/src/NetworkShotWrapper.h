#ifndef STI_ENGINE_NETWORKSHOTWRAPPER_H
#define STI_ENGINE_NETWORKSHOTWRAPPER_H

#include "Shot.h"
//#include "TShotRefInterface.h"

#include "TShot_i.h"
#include "deviceNet.h"

#include <vector>
#include <memory>

namespace STI
{
namespace Network
{




class NetworkShotWrapper : public STI::Engine::Shot
                                 //public STI::Network::TShotRefInterface	//mixin
{
public:


    NetworkShotWrapper(const std::shared_ptr<STI::Engine::Shot>& shot)
    : localshot(shot), parsedShotServant(shot)
    {
    }

    ~NetworkShotWrapper()
    {
    }

    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
    {
        if (localshot != 0) {
            localshot->getEvents(evts);
        }
    }

	static bool getTShotReference(
		const typename std::shared_ptr<STI::Engine::Shot>& shot, 
        STI::TNetwork::TShot_ptr& tShot)
	{
		auto wrapper = std::dynamic_pointer_cast<NetworkShotWrapper>(shot);
		if (wrapper) {
			tShot = wrapper->parsedShotServant._this();
			return !CORBA::is_nil(tShot);
		}
		return false;
	}

private:

    // bool getTShotRef(STI::TNetwork::TShot_ptr& tParsedShot)
    // {
    //     STI::TNetwork::TShot_var newShot;
    //     newShot = _tShot;		//implicit duplicate

    //     tParsedShot = newShot.out();
    //     return !CORBA::is_nil(tParsedShot);
    // }

    





    std::shared_ptr<STI::Engine::Shot> localshot;
//    ::STI::TNetwork::TShot_var _tShot;		//remote reference
    STI::TNetwork::TShot_i parsedShotServant;

};


} //Network
} //STI

#endif
