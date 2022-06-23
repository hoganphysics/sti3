
#include "Convert_ResultsCollector.h"
#include "RemoteResultsCollector.h"
#include "NetworkResultsCollector.h"
#include "NetworkFileHolder.h"
#include "RemoteFileHolder.h"
#include <sti/device/DeviceID.h>
#include <sti/engine/RawEvent.h>

using STI::Network::convert;
using STI::Engine::ResultsCollector;
using STI::TNetwork::TResultsCollector_var;
using STI::TNetwork::TFileHolder_var;
using STI::Utils::FileHolder;
using STI::TNetwork::TFileHolderSeq;
using STI::Network::NetworkResultsCollector;
using STI::Network::NetworkFileHolder;
using STI::Network::RemoteFileHolder;


//ResultsCollector
template<>
bool STI::Network::convert<TResultsCollector_var, std::shared_ptr<ResultsCollector>>(
        const TResultsCollector_var& tResultsCollector, std::shared_ptr<ResultsCollector>& resultsCollector)
{
    resultsCollector = std::make_shared<RemoteResultsCollector>(tResultsCollector);
    return (resultsCollector != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<ResultsCollector>, TResultsCollector_var>(
        const std::shared_ptr<ResultsCollector>& resultsCollector, TResultsCollector_var& tResultsCollector)
{
    return NetworkResultsCollector::getTResultsCollector(resultsCollector, tResultsCollector);
}


//FileHolder
template<>
bool STI::Network::convert<TFileHolder_var, std::shared_ptr<FileHolder>>(
        const TFileHolder_var& tFileHolder, std::shared_ptr<FileHolder>& fileHolder)
{
    fileHolder = std::make_shared<RemoteFileHolder>(tFileHolder);
    return (fileHolder != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<FileHolder>, TFileHolder_var>(
        const std::shared_ptr<FileHolder>& fileHolder, TFileHolder_var& tFileHolder)
{
    return NetworkFileHolder::getTFileHolderReference(fileHolder, tFileHolder);
}


//FileHolder vector 
//(vector of object reference not supported by ConvertList)
template<>
bool STI::Network::convert<std::vector<std::shared_ptr<FileHolder>>, TFileHolderSeq>(
        const std::vector<std::shared_ptr<FileHolder>>& files, TFileHolderSeq& tFiles)
{
    tFiles.length(static_cast<CORBA::ULong>(files.size()));

    for(unsigned i = 0; i < files.size(); ++i) {
        TFileHolder_var tFileHolder;
        auto& file = files.at(i);

        if (file != 0 && convert<std::shared_ptr<FileHolder>, TFileHolder_var>(file, tFileHolder)) {
            tFiles[i] = tFileHolder;
        }
    }
    return true;
}

template<>
bool STI::Network::convert<TFileHolderSeq, std::vector<std::shared_ptr<FileHolder>>>(
        const TFileHolderSeq& tFiles, std::vector<std::shared_ptr<FileHolder>>& files)
{
    unsigned len = tFiles.length();

    for(unsigned i = 0; i < len; ++i) {

        std::shared_ptr<FileHolder> fileHolder;

        if (convert<TFileHolder_var, std::shared_ptr<FileHolder>>(tFiles[i], fileHolder)) {
            files.push_back(fileHolder);
        }
    }
    return true;
}

