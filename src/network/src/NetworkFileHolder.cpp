
#include "NetworkFileHolder.h"
#include "ORBManager.h"

// #include "CerealArchives.h"
#include <sti/extern/cereal/archives/xml.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/polymorphic.hpp>

using STI::Network::NetworkFileHolder;
using STI::Utils::FileHolder;

//Serialization
CEREAL_REGISTER_TYPE(NetworkFileHolder);
CEREAL_REGISTER_POLYMORPHIC_RELATION(FileHolder, NetworkFileHolder)


NetworkFileHolder::NetworkFileHolder()
: STI::Utils::LocalFileHolder(), fileHolderServant(this)
{
}

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

