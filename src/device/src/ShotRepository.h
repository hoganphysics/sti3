
#ifndef STI_ENGINE_SHOTREPOSITORY_H
#define STI_ENGINE_SHOTREPOSITORY_H

#include <sti/fwd/Measurement_fwd.h>

//#include "ResultsDocumenter.h"

#include <memory>
#include <string>

namespace STI
{
namespace Engine
{

class ShotID;
class ParseTicket;
class ShotResult;
class ResultsCollector;

//should this be shot-specific, or general?
struct ResultsPaths
{
    std::string tempPath;
    std::string basePath;
    std::string dataPath;
    std::string timingPath;
    std::string experimentPath;
    std::string sequencePath;
};

class ShotRepository
{
public:

    virtual ~ShotRepository() {}

    //virtual std::shared_ptr<ResultsCollector> makeResultsCollector(const std::shared_ptr<ShotResult>& shotResult) = 0;

    virtual ResultsPaths preparePaths(const ShotID& sid) = 0;

    virtual bool findShot(const ShotID& sid) = 0;

    virtual bool getShot(const ShotID& id, std::shared_ptr<ShotResult>& shotResult) = 0;
    virtual bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<ShotResult>& shotResult) = 0;


    virtual bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements) = 0;
    //virtual bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements, const std::function<bool(const Measurement&)>& selector) = 0;

    //virtual bool getParseTicket(const ShotID& sid, std::shared_ptr<ParseTicket>& parseTicket) = 0;    //associated ParseTicket has events, etc.

};


} //Engine
} //STI

#endif

