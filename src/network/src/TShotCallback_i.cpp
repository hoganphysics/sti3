#include "TShotCallback_i.h"

#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEvent.h>

#include "convert/Convert_EventEngine.h"
#include "convert/Convert_ShotResult.h"
#include "convert/Convert_RawEventGroup.h"
#include "generated/orbTypes.h"

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
}

void TShotCallback_i::getRootEventGroup(::STI::TNetwork::TRawEventGroup_out rootGroup)
{
    rootGroup = new STI::TNetwork::TRawEventGroup();
    
    if (localShot != 0) {
        std::shared_ptr<STI::Engine::RawEventGroup> pGroup;
        localShot->getRootEventGroup(pGroup);

        STI::TNetwork::TRawEventGroup_var tRawEventGroup_var(new STI::TNetwork::TRawEventGroup);

        if (pGroup != 0 && convert<std::shared_ptr<STI::Engine::RawEventGroup>, STI::TNetwork::TRawEventGroup>(pGroup, tRawEventGroup_var)) {
            //success
            (*rootGroup) = tRawEventGroup_var;
        }
    }
}
