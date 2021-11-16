
#include "SerializedRepository.h"
#include "ShotID.h"
#include "LocalResultsCollector.h"
#include "Measurement.h"
#include "RawEvent.h"
#include "ShotResult.h"
// #include "TimeStamp.h"

#include "CerealArchives.h"

#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>


#include <filesystem>
#include <iostream>
#include <fstream>

using STI::Engine::SerializedRepository;
using STI::Engine::ResultsPaths;
using STI::Engine::ShotID;
using STI::Engine::LocalResultsCollector;


SerializedRepository::SerializedRepository(const std::string& baseDevicePath)
: cachedPaths(5), baseDevicePath(baseDevicePath)
{
    // baseDevicePath = makeBaseDevicePath();

    archiveFilename = "out.xml";
}


bool SerializedRepository::findShot(const ShotID& sid)
{
    std::filesystem::path shotPath = getShotBasePath(sid);

    return std::filesystem::exists(shotPath);
}

bool SerializedRepository::getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements)
{
    if (!findShot(sid)) return false;

    return false;
}

bool SerializedRepository::getParseTicket(const ShotID& sid, std::shared_ptr<ParseTicket>& parseTicket)
{
    if (!findShot(sid)) return false;

    return false;
}


ResultsPaths SerializedRepository::preparePaths(const ShotID& sid)
{
    ResultsPaths paths = makePaths(sid);

    makePathIfNew(paths.basePath);
    makePathIfNew(paths.tempPath);
    makePathIfNew(paths.dataPath);
    makePathIfNew(paths.timingPath);
    makePathIfNew(paths.experimentPath);
    makePathIfNew(paths.sequencePath);

    return paths;
}

void SerializedRepository::makePathIfNew(const std::string& pathName)
{
    std::filesystem::path newPath = pathName;
    if (!std::filesystem::exists(newPath)) {
        std::filesystem::create_directories(newPath);
    }

}

// template<class Archive>
// void serialize(Archive& archive, STI::Device::DeviceID& m)
// {
//   archive( m., m.y, m.z );
// }

// template<class Archive>
// void serialize(Archive& archive, ShotID& shotID)
// {
//     archive( cereal::make_nvp("ParseID", shotID.parseID),
//             cereal::make_nvp("SubmissionTime", shotID.submissionTime),
//             cereal::make_nvp("PlayTime", shotID.playTime),
//             cereal::make_nvp("JobSourceID", shotID.jobSourceID) );
// }

// tm timeinfo;


bool SerializedRepository::saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<ShotResult>& shotResult)
//bool SerializedRepository::save(const ResultsPaths& paths, const std::shared_ptr<LocalResultsCollector>& resultsCollector)
{
    // auto measurements = resultsCollector->getMeasurements();

    // if (measurements != 0) {
    //     for(auto& meas : *measurements) {
    //         std::cout << "Meas: " << meas->data().print() << std::endl;
    //     }
    // }

    auto paths = preparePaths(sid);

    std::filesystem::path serializePath = paths.dataPath;
    serializePath /= archiveFilename;

    
    {
        std::ofstream file( serializePath.string() );
        cereal::XMLOutputArchive archive( file );

        //auto results = resultsCollector->getResults();

        // STI::Device::DeviceID id("test", "localhost", 2);

        //archive(id);
        //archive( results );
        archive( shotResult );

        // archive( 
        //     cereal::make_nvp("ShotID", results->sid),
        //     cereal::make_nvp("Attributes", results->attributes), 
        //     cereal::make_nvp("Measurements", results->measurements),
        //     cereal::make_nvp("timingFiles", results->timingFiles),
        //      cereal::make_nvp("parsedEvents", results->parsedEvents)
        //  );

         

        //During load(), need to call fileFactory to create new file references (wraps files in NetworkFile as needed)

        //need to recurvisely re make all FileHolder in each MixedValue
        // void replace(FileHolder&, FileHolderFactory&) {
        //     if (vector) {
        //         for each f {
        //             repalce(f, factory);
        //         }
        //     }
        // }

        //archive( resultsCollector );
    }

    //test

    // std::shared_ptr<STI::Engine::ShotResult> testResult;

    // getShot( resultsCollector->getShotID(), testResult);

    // std::cout << "Test result: " << testResult->measurements->at(0)->data().print() << std::endl;

    return true;
}



bool SerializedRepository::getShot(const ShotID& sid, std::shared_ptr<ShotResult>& shotResult)
//bool SerializedRepository::load(const ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& shotResult)
{
    if (!findShot(sid)) return false;

    auto paths = preparePaths(sid);

    std::filesystem::path serializePath = paths.dataPath;
    serializePath /= archiveFilename;
    {
        std::ifstream file( serializePath.string() );
        cereal::XMLInputArchive archive( file );  
        
        shotResult = std::make_shared<STI::Engine::ShotResult>();

        archive(shotResult);
    }

    return true;
}


ResultsPaths SerializedRepository::makePaths(const ShotID& sid)
{
    std::unique_lock<std::mutex> pathLock(pathMutex);

    ResultsPaths paths;

    if (cachedPaths.get(sid, paths)) {
        return paths;
    }

    // std::filesystem::path basePath(baseDevicePath);

    // basePath /= sid.playTime.date_YYYY_MM_DD();

    // if (!std::filesystem::exists(basePath)) {
    //     std::filesystem::create_directories(basePath);
    // }


    // std::string shotTime = sid.playTime.time_hh_mm_ss();

    // //Make unique directory, using shot time as base
    // unsigned unique = 1;
    // auto uniqueBasePath = basePath;
    // while (std::filesystem::exists(uniqueBasePath))
    // {
    //     unique++;
    //     uniqueBasePath = basePath;
    //     uniqueBasePath /= shotTime  + "_" + std::to_string(unique);
    // }

    // auto uniqueBasePath = basePath / sid.playTime.time_hh_mm_ss_mmmuuunnn();
    // std::filesystem::create_directory(uniqueBasePath);


    std::filesystem::path uniqueBasePath = baseDevicePath;
    uniqueBasePath /= "shot_cache";
    uniqueBasePath /= sid.playTime.date_YYYY_MM_DD();
    uniqueBasePath /= sid.playTime.time_hh_mm_ss_mmmuuunnn();       //shots stored by timestamp

    paths.basePath = uniqueBasePath.string();

    auto tempPath = uniqueBasePath / "temp";
    paths.tempPath = tempPath.string();

    auto dataPath = uniqueBasePath / "data";
    paths.dataPath = dataPath.string();

    auto timingPath = uniqueBasePath / "timing";
    paths.timingPath = timingPath.string();

    auto experimentPath = uniqueBasePath / "experiments";
    paths.experimentPath = experimentPath.string();

    auto sequencePath = uniqueBasePath / "sequences";
    paths.sequencePath = sequencePath.string();

    cachedPaths.add(sid, paths);

    return paths;
}

std::string SerializedRepository::getShotBasePath(const ShotID& sid)
{
    std::unique_lock<std::mutex> pathLock(pathMutex);

    std::filesystem::path basePath(baseDevicePath);

    basePath /= sid.playTime.date_YYYY_MM_DD();
    auto uniqueBasePath = basePath / sid.playTime.time_hh_mm_ss_mmmuuunnn();

    return uniqueBasePath.string();
}

// std::string SerializedRepository::makeBaseDevicePath()
// {
//     std::filesystem::path root(rootPath);

//     if (!std::filesystem::exists(root)) {
//         std::filesystem::create_directory(root);
//     }

//     auto devicePath = root / deviceID.getID();

//     if (!std::filesystem::exists(devicePath)) {
//         std::filesystem::create_directories(devicePath);
//     }

//     return devicePath.string();
// }
