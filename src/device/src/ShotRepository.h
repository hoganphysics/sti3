#ifndef STI_ENGINE_SHOTREPOSITORY_H
#define STI_ENGINE_SHOTREPOSITORY_H

#include <sti/fwd/Measurement_fwd.h>

#include <memory>
#include <string>


namespace STI
{
namespace Engine
{

class ShotID;
class ParseTicket;
class ShotResult;
class ParseResult;
class ResultsCollector;
class FullShotResult;
class ParseID;

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

    virtual bool findShotResult(const ShotID& sid) = 0;
    virtual bool findParseResult(const ParseID& sid) = 0;

    virtual bool getShotResult(const ShotID& id, std::shared_ptr<ShotResult>& shotResult) = 0;
    virtual bool getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& shotResult) = 0;

    virtual bool saveShot(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult) = 0;


    virtual bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements) = 0;
    //virtual bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements, const std::function<bool(const Measurement&)>& selector) = 0;

    //virtual bool getParseTicket(const ShotID& sid, std::shared_ptr<ParseTicket>& parseTicket) = 0;    //associated ParseTicket has events, etc.

};


} //Engine
} //STI

#endif

