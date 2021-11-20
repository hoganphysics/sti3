
#include "TShotEventsCallback_i.h"

#include "ORBManager.h"
#include "RawEvent.h"
#include "Convert_EventEngine.h"

#include "orbTypes.h"

#include <vector>
#include <memory>

using STI::TNetwork::TShotEventsCallback_i;
using STI::Network::convert;


TShotEventsCallback_i::TShotEventsCallback_i(const std::shared_ptr<STI::Engine::Shot>& shot)
: localShot(shot)
{
}

TShotEventsCallback_i::~TShotEventsCallback_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TShotEventsCallback_i::getEvents(::STI::TNetwork::TRawEventSeq_out events)
{
    // if (localShot != 0) {
    //     std::shared_ptr<STI::Engine::RawEventVector> evts;
    //     localShot->getEvents(evts);

    //     STI::TNetwork::TRawEventSeq_var tRawEvtseq_var(new STI::TNetwork::TRawEventSeq);

    //     if (evts != 0 && convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(*evts, 
    //         (_CORBA_Unbounded_Sequence<STI::TNetwork::TRawEvent>&)tRawEvtseq_var)) {
    //         events = tRawEvtseq_var.out();
    //     }
    // }

    if (localShot != 0) {
        std::shared_ptr<STI::Engine::RawEventVector> evts;
        localShot->getEvents(evts);

        STI::TNetwork::TRawEventSeq_var tRawEvtseq_var(new STI::TNetwork::TRawEventSeq);

        events = new STI::TNetwork::TRawEventSeq();
        
        //events = tRawEvtseq_var.out();
        if (evts != 0 && convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(*evts, 
            (_CORBA_Unbounded_Sequence<STI::TNetwork::TRawEvent>&) tRawEvtseq_var)) {
                //success
                (*events) = tRawEvtseq_var;
        }


        // if (evts != 0 && convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(*evts, 
        //     (_CORBA_Unbounded_Sequence<STI::TNetwork::TRawEvent>&) events)) {
        //         //success
        // }
    }

}

