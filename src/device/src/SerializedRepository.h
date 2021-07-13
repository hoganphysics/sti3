
#ifndef STI_ENGINE_SERIALIZEDREPOSITORY_H
#define STI_ENGINE_SERIALIZEDREPOSITORY_H

#include "ShotRepository.h"
#include "ResultsDocumenter.h"
#include "DeviceID.h"
#include "utils/OrderedBufferMap.h"

#include <memory>
#include <string>
#include <mutex>


namespace STI
{
namespace Engine
{


class SerializedRepository : public ShotRepository, 
                             public ResultsDocumenter
{
public:

    SerializedRepository(const std::string& rootPath, const STI::Device::DeviceID& deviceID);

    //ShotRepositroy
    bool findShot(const ShotID& sid);
    bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements);
    bool getParseTicket(const ShotID& sid, std::shared_ptr<ParseTicket>& parseTicket); 

    //ResultsDocumenter
    ResultsPaths preparePaths(const ShotID& sid);
    bool save(const ResultsPaths& paths, const std::shared_ptr<LocalResultsCollector>& resultsCollector);

private:

    ResultsPaths makePaths(const ShotID& sid);
    std::string makeBaseDevicePath();

    void makePathIfNew(const std::string& pathName);

    std::string getShotBasePath(const ShotID& sid);

    STI::Device::DeviceID deviceID;
    std::string rootPath;
    std::string baseDevicePath;

    STI::Utils::OrderedBufferMap<ShotID, ResultsPaths> cachedPaths;

    mutable std::mutex pathMutex;

};


} //Engine
} //STI

#endif





