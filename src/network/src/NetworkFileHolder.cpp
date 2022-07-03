
#include "NetworkFileHolder.h"
#include "ORBManager.h"

using STI::Network::NetworkFileHolder;



NetworkFileHolder::NetworkFileHolder(const std::string& filename)
: STI::Utils::LocalFileHolder(filename), fileHolderServant(this)
{
     STI::Network::ORBManager::ORBManager::activateServant(fileHolderServant);
}

NetworkFileHolder::~NetworkFileHolder()
{
}


bool NetworkFileHolder::getTFileHolderRef(STI::TNetwork::TFileHolder_var& tFileHolder)
{
    tFileHolder = fileHolderServant._this();
    return !CORBA::is_nil(tFileHolder);
}

