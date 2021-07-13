
#include "LocalResultsCollector.h"
#include "ShotID.h"
#include "EventEngine.h"
#include "ParsedDependencyTree.h"
#include "Measurement.h"
#include "RawEvent.h"

#include <filesystem>
namespace fs = std::filesystem;

using STI::Engine::LocalResultsCollector;
using STI::Engine::ShotID;
using STI::Engine::ParsedDependencyTree;
using STI::Engine::Measurement;
using STI::Engine::MeasurementVector;


LocalResultsCollector::LocalResultsCollector(const ShotID& shotID, 
                    const std::shared_ptr<STI::Engine::EventEngine>& eventEngine, 
                    const std::shared_ptr<ParsedDependencyTree>& dependencies,
                    const ResultsPaths& paths,
                    const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
: sid(shotID), eventEngine(eventEngine), dependencies(dependencies), resultsPaths(paths), fileHolderFactory(factory)
{
    // resultTicket = std::make_shared<STI::Engine::ResultTicket>(shotID, );
}


ShotID LocalResultsCollector::getShotID()
{
    return sid;
}

std::shared_ptr<ParsedDependencyTree> LocalResultsCollector::getDependencies()
{
    return dependencies;
}


void LocalResultsCollector::addEvents(const DeviceEventMap& parsedEvents)
{
    // resultsTicket.events = parsedEvents;
    parsedEvents_ = std::move(parsedEvents);
}

void LocalResultsCollector::addTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files)
{
    for (auto& file : files) {
        std::string localPath = makeLocalPath(resultsPaths.timingPath, file->getFilename());
        auto localFileHandle = fileHolderFactory->makeFileHolder(localPath);

        if (file->transferFile(localFileHandle)) {
        }
    }
}


bool LocalResultsCollector::addMeasurements(const std::shared_ptr<MeasurementVector>& measurements)
{
    if (measurements == 0) return false;

    measurements_ = measurements;

    bool success = true;

    //on results destination
    STI::Utils::MixedValue filedata;

    for (auto& meas : *measurements) {
        if (meas != 0 && meas->data().getType() == STI::Utils::MixedValueType::File) {
            //possibly do this in 
            meas->extractMeasurementResult(filedata);
            auto fileHandle = filedata.getFile();

            // std::string localPath = resultsDocumenter->makeLocalPath(sid, fileHandle->getFilename());
            
            std::string localPath = makeLocalPath(resultsPaths.dataPath, fileHandle->getFilename());
            
            auto localFileHandle = fileHolderFactory->makeFileHolder(localPath);
            
            //transfer file to local
            if (fileHandle->transferFile(localFileHandle)) {
                //sucess; delete remote?
                success &= true;
                filedata.setValue(localFileHandle);
                meas->setMeasurementResult(filedata);
            }
            else {
                success = false;
            }
        }

    }
    return success;
}

std::shared_ptr<MeasurementVector> LocalResultsCollector::getMeasurements()
{
    return measurements_;
}

std::string LocalResultsCollector::makeLocalPath(const std::string& basePath, const std::string& remoteFilename)
{
    fs::path remotePath = remoteFilename;
    fs::path localPath = basePath;

    localPath /= remotePath.filename();

    return makeUniquePath( localPath.string() );
}

std::string LocalResultsCollector::makeUniquePath(const std::string& filename)
{
    fs::path initialPath = filename;

    fs::path trialPath = initialPath;
    unsigned n = 0;

    while (fs::exists(trialPath)) {
        n++;
        trialPath = initialPath.parent_path();
        trialPath /= initialPath.stem();
        trialPath /= "_";
        trialPath /= std::to_string(n);
        trialPath.replace_extension( initialPath.extension() );
    }

    return trialPath.string();
}

bool LocalResultsCollector::addAttributes(const STI::Device::DeviceID& deviceID, const std::vector<std::shared_ptr<STI::Device::Attribute>>& attributes)
{
    return false;
}