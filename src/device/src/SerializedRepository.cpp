
#include "SerializedRepository.h"
#include "ShotID.h"
#include "LocalResultsCollector.h"
#include "Measurement.h"
#include "RawEvent.h"

#include <filesystem>
#include <iostream>

using STI::Engine::SerializedRepository;
using STI::Engine::ResultsPaths;
using STI::Engine::ShotID;
using STI::Engine::LocalResultsCollector;


SerializedRepository::SerializedRepository(const std::string& rootPath, const STI::Device::DeviceID& deviceID)
: cachedPaths(5), deviceID(deviceID), rootPath(rootPath)
{
    baseDevicePath = makeBaseDevicePath();
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

bool SerializedRepository::save(const ResultsPaths& paths, const std::shared_ptr<LocalResultsCollector>& resultsCollector)
{
    auto measurements = resultsCollector->getMeasurements();

    if (measurements != 0) {
        for(auto& meas : *measurements) {
            std::cout << "Meas: " << meas->data().print() << std::endl;
        }
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


    std::filesystem::path uniqueBasePath = makeBaseDevicePath();

    
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

std::string SerializedRepository::makeBaseDevicePath()
{
    std::filesystem::path root(rootPath);

    if (!std::filesystem::exists(root)) {
        std::filesystem::create_directory(root);
    }

    auto devicePath = root / deviceID.getID();

    if (!std::filesystem::exists(devicePath)) {
        std::filesystem::create_directories(devicePath);
    }

    return devicePath.string();
}
