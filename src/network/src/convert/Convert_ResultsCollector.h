
#ifndef STI_NETWORK_CONVERT_RESULTSCOLLECTOR_H
#define STI_NETWORK_CONVERT_RESULTSCOLLECTOR_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include "generated/orbTypes.h"

#include <memory>
#include <vector>


namespace STI
{

namespace Engine
{

class ResultsCollector;

} //Engine

namespace Utils
{

class FileHolder;

} //Utils



//ResultsCollector
template<>
bool Network::convert<TNetwork::TResultsCollector_var, std::shared_ptr<Engine::ResultsCollector>>(
        const TNetwork::TResultsCollector_var& tResultsCollector, std::shared_ptr<Engine::ResultsCollector>& resultsCollector);
template<>
bool Network::convert<std::shared_ptr<Engine::ResultsCollector>, TNetwork::TResultsCollector_var>(
        const std::shared_ptr<Engine::ResultsCollector>& resultsCollector, TNetwork::TResultsCollector_var& tResultsCollector);


//FileHolder
template<>
bool Network::convert<TNetwork::TFileHolder_var, std::shared_ptr<Utils::FileHolder>>(
        const TNetwork::TFileHolder_var& tFileHolder, std::shared_ptr<Utils::FileHolder>& fileHolder);
template<>
bool Network::convert<std::shared_ptr<Utils::FileHolder>, TNetwork::TFileHolder_var>(
        const std::shared_ptr<Utils::FileHolder>& fileHolder, TNetwork::TFileHolder_var& tFileHolder);


//FileHolder vector 
//(vector of object reference not supported by ConvertList)
template<>
bool Network::convert<std::vector<std::shared_ptr<Utils::FileHolder>>, TNetwork::TFileHolderSeq>(
        const std::vector<std::shared_ptr<Utils::FileHolder>>& files, TNetwork::TFileHolderSeq& tFiles);
template<>
bool Network::convert<TNetwork::TFileHolderSeq, std::vector<std::shared_ptr<Utils::FileHolder>>>(
        const TNetwork::TFileHolderSeq& tFiles, std::vector<std::shared_ptr<Utils::FileHolder>>& files);


} //STI

#endif

