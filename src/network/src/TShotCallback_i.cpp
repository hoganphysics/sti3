
#include "TShotCallback_i.h"

#include "ORBManager.h"
#include <sti/engine/RawEvent.h>
#include "Convert_EventEngine.h"
#include <sti/engine/ParseResult.h>

#include "Convert_ShotResult.h"
#include "Convert_RawEventGroup.h"

#include "orbTypes.h"

#include <vector>
#include <memory>

using STI::TNetwork::TShotCallback_i;
using STI::Network::convert;


TShotCallback_i::TShotCallback_i(const std::shared_ptr<STI::Engine::Shot>& shot)
: localShot(shot)
{
}

TShotCallback_i::~TShotCallback_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

// void TShotCallback_i::getGroups(::STI::TNetwork::TRawEventGroup_out groups)
// {
//     if (localShot != 0) {
//         std::vector<STI::Engine::RawEventGroup> eventGroups;
//         localShot->getGroups(eventGroups);

//         STI::TNetwork::TRawEventGroupSeq_var tRawEvtGroupSeq_var(new STI::TNetwork::TRawEventGroupSeq);

//         groups = new STI::TNetwork::TRawEventGroupSeq();
        
//         if (convert<STI::Engine::RawEventGroup, STI::TNetwork::TRawEventGroup>(eventGroups, 
//             (_CORBA_Unbounded_Sequence<STI::TNetwork::TRawEventGroup>&) tRawEvtGroupSeq_var)) {
//                 //success
//                 (*groups) = tRawEvtGroupSeq_var;
//         }
//     }
// }



// void TShotCallback_i::getVars(::STI::TNetwork::TParsedVarSeq_out parsedVars)
// {
//     if (localShot != 0) {
//         std::vector<STI::Engine::ParsedVar> vars;
//         localShot->getParsedVars(vars);

//         STI::TNetwork::TParsedVarSeq_var tParsedVarSeq_var(new STI::TNetwork::TParsedVarSeq);

//         parsedVars = new STI::TNetwork::TRawEventGroupSeq();
        
//         if (convert<STI::Engine::ParsedVar, STI::TNetwork::TParsedVar>(vars, 
//             (_CORBA_Unbounded_Sequence<STI::TNetwork::TParsedVar>&) tParsedVarSeq_var)) {
//                 //success
//                 (*parsedVars) = tParsedVarSeq_var;
//         }
//     }
// }

// void TShotCallback_i::getTags(::STI::TNetwork::TParsedTagSeq_out parsedTags)
// {
//     if (localShot != 0) {
//         std::vector<STI::Engine::ParsedTag> tags;
//         localShot->getParsedVars(vars);

//         STI::TNetwork::TParsedTagSeq_var tParsedTagSeq_var(new STI::TNetwork::TParsedTagSeq);

//         parsedTags = new STI::TNetwork::TRawEventGroupSeq();
        
//         if (convert<STI::Engine::ParsedVar, STI::TNetwork::TParsedTag>(tags, 
//             (_CORBA_Unbounded_Sequence<STI::TNetwork::TParsedTag>&) tParsedTagSeq_var)) {
//                 //success
//                 (*parsedTags) = tParsedTagSeq_var;
//         }
//     }
// }

// void TShotCallback_i::getParseResult(::STI::TNetwork::TParseResult_out parseResult)
// {
//     if (localShot != 0) {
//         std::shared_ptr<STI::Engine::ParseResult> pResult;
//         localShot->getParseResult(pResult);

//         STI::TNetwork::TParseResult_var tParseResult_var(new STI::TNetwork::TParseResult);

//         parseResult = new STI::TNetwork::TParseResult();
        
//         //events = tRawEvtseq_var.out();
//         if (pResult != 0 && convert<STI::Engine::ParseResult, STI::TNetwork::TParseResult>(*pResult, tParseResult_var)) {
//                 //success
//                 (*parseResult) = tParseResult_var;
//         }
//     }
// }

// void TShotCallback_i::getEvents(::STI::TNetwork::TRawEventSeq_out events)
// {
//     // if (localShot != 0) {
//     //     std::shared_ptr<STI::Engine::RawEventVector> evts;
//     //     localShot->getEvents(evts);

//     //     STI::TNetwork::TRawEventSeq_var tRawEvtseq_var(new STI::TNetwork::TRawEventSeq);

//     //     if (evts != 0 && convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(*evts, 
//     //         (_CORBA_Unbounded_Sequence<STI::TNetwork::TRawEvent>&)tRawEvtseq_var)) {
//     //         events = tRawEvtseq_var.out();
//     //     }
//     // }

//     if (localShot != 0) {
//         std::shared_ptr<STI::Engine::RawEventVector> evts;
//         localShot->getEvents(evts);

//         STI::TNetwork::TRawEventSeq_var tRawEvtseq_var(new STI::TNetwork::TRawEventSeq);

//         events = new STI::TNetwork::TRawEventSeq();
        
//         //events = tRawEvtseq_var.out();
//         if (evts != 0 && convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(*evts, 
//             (_CORBA_Unbounded_Sequence<STI::TNetwork::TRawEvent>&) tRawEvtseq_var)) {
//                 //success
//                 (*events) = tRawEvtseq_var;
//         }


//         // if (evts != 0 && convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(*evts, 
//         //     (_CORBA_Unbounded_Sequence<STI::TNetwork::TRawEvent>&) events)) {
//         //         //success
//         // }
//     }

// }


void TShotCallback_i::getBaseEventGroup(::STI::TNetwork::TRawEventGroup_out baseGroup)
{
    if (localShot != 0) {
        std::shared_ptr<STI::Engine::RawEventGroup> pGroup;
        localShot->getBaseEventGroup(pGroup);

        STI::TNetwork::TRawEventGroup_var tRawEventGroup_var(new STI::TNetwork::TRawEventGroup);

        baseGroup = new STI::TNetwork::TRawEventGroup();

        if (pGroup != 0 && convert<std::shared_ptr<STI::Engine::RawEventGroup>, STI::TNetwork::TRawEventGroup>(pGroup, tRawEventGroup_var)) {
            //success
            (*baseGroup) = tRawEventGroup_var;
        }
    }
}
