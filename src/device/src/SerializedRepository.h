
#ifndef STI_ENGINE_SERIALIZEDREPOSITORY_H
#define STI_ENGINE_SERIALIZEDREPOSITORY_H

#include "ShotRepository.h"
//#include "ResultsDocumenter.h"
#include "DeviceID.h"
#include "utils/OrderedBufferMap.h"

#include <memory>
#include <string>
#include <mutex>


namespace STI
{
namespace Engine
{

class ShotResult;

class SerializedRepository : public ShotRepository
                           //  public ResultsDocumenter
{
public:

    SerializedRepository(const std::string& baseDevicePath);


    //ShotRepositroy
    bool findShot(const ShotID& sid);
    bool getShot(const ShotID& id, std::shared_ptr<ShotResult>& shotResult);
    bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<ShotResult>& shotResult);

    bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements);
    bool getParseTicket(const ShotID& sid, std::shared_ptr<ParseTicket>& parseTicket); 

    //ResultsDocumenter
    ResultsPaths preparePaths(const ShotID& sid);
    // bool save(const ResultsPaths& paths, const std::shared_ptr<LocalResultsCollector>& resultsCollector);

    // bool load(const ShotID& sid, std::shared_ptr<ShotResult>& shotResult);

private:

    ResultsPaths makePaths(const ShotID& sid);
    // std::string makeBaseDevicePath();

    void makePathIfNew(const std::string& pathName);

    std::string getShotBasePath(const ShotID& sid);

    // STI::Device::DeviceID deviceID;
    std::string rootPath;
    std::string baseDevicePath;

    std::string archiveFilename;

    STI::Utils::OrderedBufferMap<ShotID, ResultsPaths> cachedPaths;

    mutable std::mutex pathMutex;

};


} //Engine
} //STI

#endif





