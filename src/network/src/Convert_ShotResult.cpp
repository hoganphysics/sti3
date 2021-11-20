
#include "Convert_ShotResult.h"
#include "Convert_EventEngine.h"
#include "Convert_Attribute.h"
#include "Convert_ResultsCollector.h"

#include "ShotResult.h"
#include "RawEvent.h"

#include <memory>

using STI::Network::convert;

using STI::Engine::ShotResult;
using STI::TNetwork::TShotResult;
using STI::TNetwork::TTimeStamp;
using STI::Engine::TimeStamp;
using STI::TNetwork::TShotResultRecord;
using STI::Engine::ShotResultRecord;


//ShotResult
template<>
bool STI::Network::convert<TShotResult, std::shared_ptr<ShotResult>>(
        const TShotResult& tShotResult, std::shared_ptr<ShotResult>& shotResult)
{
    shotResult = std::make_shared<ShotResult>();

    shotResult->sid = convert<TNetwork::TShotID, Engine::ShotID>(tShotResult.sid);
    shotResult->playTime = convert<TTimeStamp, TimeStamp>(tShotResult.playTime);

    convert<::STI::TNetwork::TDeviceEventsSeq, STI::Engine::DeviceEventMap>(tShotResult.parsedEvents, shotResult->parsedEvents);
    convert<STI::TNetwork::TFileHolderSeq, std::vector<std::shared_ptr<STI::Utils::FileHolder>>>(tShotResult.timingFiles, shotResult->timingFiles);

    shotResult->measurements = std::make_shared<STI::Engine::MeasurementVector>();
    convert<STI::TNetwork::TMeasurement, std::shared_ptr<STI::Engine::Measurement>>(tShotResult.measurements, *(shotResult->measurements));

    for (unsigned i = 0; i < tShotResult.attributes.length(); ++i) {

        convert<::STI::TNetwork::TAttributeTupleSeq, std::map<std::string, std::string>>(
                tShotResult.attributes[i].attributes,
                (shotResult->attributes)[convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tShotResult.attributes[i].id)]
            );
    }

    convert<TShotResultRecord, ShotResultRecord>(tShotResult.shotResultRecord, shotResult->shotResultRecord);

    return true;
}

template<>
bool STI::Network::convert<std::shared_ptr<ShotResult>, TShotResult>(
        const std::shared_ptr<ShotResult>& shotResult, TShotResult& tShotResult)
{
    if (shotResult == 0) return false;

    tShotResult.sid = convert<Engine::ShotID, TNetwork::TShotID>(shotResult->sid);
    tShotResult.playTime = convert<TimeStamp, TTimeStamp>(shotResult->playTime);

    convert<STI::Engine::DeviceEventMap, ::STI::TNetwork::TDeviceEventsSeq>(shotResult->parsedEvents, tShotResult.parsedEvents);
    convert<std::vector<std::shared_ptr<STI::Utils::FileHolder>>, STI::TNetwork::TFileHolderSeq>(shotResult->timingFiles, tShotResult.timingFiles);

    if (shotResult->measurements != 0) {
        convert<std::shared_ptr<STI::Engine::Measurement>, STI::TNetwork::TMeasurement>(*(shotResult->measurements), tShotResult.measurements);        
    }

    tShotResult.attributes.length( shotResult->attributes.size() );
    unsigned i = 0;
    for (auto& deviceAttributes : shotResult->attributes) {
        
        tShotResult.attributes[i].id = convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(deviceAttributes.first);

        convert<std::map<std::string, std::string>, ::STI::TNetwork::TAttributeTupleSeq>(
                deviceAttributes.second, 
                tShotResult.attributes[i].attributes);
        i++;
    }

    convert<ShotResultRecord, TShotResultRecord>(shotResult->shotResultRecord, tShotResult.shotResultRecord);

    return true;
}

