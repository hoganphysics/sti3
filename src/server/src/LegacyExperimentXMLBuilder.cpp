
#include "LegacyExperimentXMLBuilder.h"

#include <sti/engine/ShotResult.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/StackTraceResult.h>
#include <sti/engine/StackTraceData.h>

#include <map>
#include <string>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iterator>


using STI::Engine::LegacyExperimentXMLBuilder;
using STI::Engine::FullShotResult;
using STI::Device::DeviceID;

LegacyExperimentXMLBuilder::LegacyExperimentXMLBuilder()
{
}

LegacyExperimentXMLBuilder::LegacyExperimentXMLBuilder(const std::string& filename, const std::shared_ptr<FullShotResult>& fullShotResult)
: filename(filename), fullShotResult(fullShotResult)
{
    // build(filename, fullShotResult);
}

LegacyExperimentXMLBuilder::~LegacyExperimentXMLBuilder()
{
}


LegacyExperimentXMLBuilder& LegacyExperimentXMLBuilder::setShotResult(const std::string& shotFilename, const std::shared_ptr<FullShotResult>& shotResult)
{
    filename = shotFilename;
    fullShotResult = shotResult;
    return *this;
}

LegacyExperimentXMLBuilder& LegacyExperimentXMLBuilder::addSequenceFilename(const std::string& seqFilename)
{
    sequenceFilename = seqFilename;
    return *this;
}

void addAttributes(tinyxml2::XMLElement* attributes, const std::map<std::string, std::string>& attributeMap)
{
    for (auto& attrib : attributeMap) {
        auto attribute = attributes->InsertNewChildElement("attribute");
        attribute->SetAttribute("key", attrib.first.c_str());
        attribute->SetAttribute("value", attrib.second.c_str());
    }
}

template<class T>
void addDimlimitedVector(tinyxml2::XMLElement* base, const std::vector<T>* values, const std::string& delimiter, const std::string& type)
{
    if (values == 0) return;

    auto vec = base->InsertNewChildElement("delimitedvector");
    vec->SetAttribute("delimiter", delimiter.c_str());
    vec->SetAttribute("length", STI::Utils::valueToString(values->size()).c_str());
    vec->SetAttribute("type", type.c_str());

    std::ostringstream out;
    if (!values->empty())
    {
        std::copy(std::begin(*values), std::end(*values) - 1, std::ostream_iterator<T>(out, delimiter.c_str()));
        out << values->back();
    }
    vec->SetText(out.str().c_str());
}

void LegacyExperimentXMLBuilder::addValue(tinyxml2::XMLElement* base, const STI::Utils::MixedValue& value)
{
    using STI::Utils::MixedValueType;

    switch(value.getType()) {
        case MixedValueType::Any:
            break;
        case MixedValueType::Boolean:
            base->InsertNewChildElement("bool")
                ->SetText(value.getBoolean());
            break;
        case MixedValueType::Double:
            base->InsertNewChildElement("double")
                ->SetText(value.getDouble());
            break;
        case MixedValueType::Empty:
            break;
        case MixedValueType::File:
            {
                auto file = base->InsertNewChildElement("file");
                if (value.getFile() != 0) {
                    file->InsertNewChildElement("filename")->SetText(value.getFile()->getFilename().c_str());
                    file->InsertNewChildElement("md5hash")->SetText(value.getFile()->md5Checksum().c_str());
                }
            }
            break;
        case MixedValueType::Image:
            break;
        case MixedValueType::Int:
            base->InsertNewChildElement("int")
                ->SetText(value.getInt());
            break;
        case MixedValueType::String:
            base->InsertNewChildElement("string")
                ->SetText(value.getString().c_str());
            break;
        case MixedValueType::Vector:
            {
                auto vec = base->InsertNewChildElement("vector");
                for (auto& v : value.getVector()) {
                    addValue(vec, v);
                }                
            }
            break;
        case MixedValueType::VectorInt:
            {
                const std::vector<int>* values;
                if (value.getFlatVector(values)) {

                    addDimlimitedVector(base, values, ",", "Int");

                    // std::string delimiter = ",";

                    // auto vec = base->InsertNewChildElement("delimitedvector");
                    // vec->SetAttribute("delimiter", delimiter.c_str());
                    // vec->SetAttribute("length", STI::Utils::valueToString(values->size()).c_str());
                    // vec->SetAttribute("type", "Int");

                    // std::ostringstream out;
                    // if (!values->empty())
                    // {
                    //     std::copy(std::begin(*values), std::end(*values) - 1, std::ostream_iterator<int>(out, delimiter.c_str()));
                    //     out << values->back();
                    // }
                    // vec->SetText(out.str().c_str());
                }
            }
            break;
    }
}

void addMeasurements(tinyxml2::XMLElement* measurements, const STI::Engine::MeasurementVector& measurementVector)
{
    for (auto& meas : measurementVector) {
        //meas
        if (meas != 0) {
            auto measurement = measurements->InsertNewChildElement("measurement");
            measurement->SetAttribute("time", meas->time());
            measurement->SetAttribute("channel", meas->channel());
            measurement->SetAttribute("group", meas->groupName().c_str());
            LegacyExperimentXMLBuilder::addValue(measurement, meas->data());
        }
    }
}

void addVars(tinyxml2::XMLElement* timing, const std::shared_ptr<STI::Engine::RawEventGroup>& group)
{
    if (group == 0) return;

    auto vars = group->getVars();
    auto ovars = group->getOverwrittenVars();
    for (auto& var : vars) {
        auto varElement = timing->InsertNewChildElement("var");
        varElement->SetAttribute("name", var.name.c_str());

        auto it = std::find_if(ovars.begin(), ovars.end(), [&var](auto& ovar) { return ovar.name.compare(var.name) == 0; });
        varElement->SetAttribute("overwritten", it != ovars.end());

        LegacyExperimentXMLBuilder::addValue(varElement, var.value);
    }

    
    // for (auto& ovar : ovars) {
    //     auto varElement = timing->InsertNewChildElement("var");
    //     varElement->SetAttribute("name", ovar.name.c_str());
    //     varElement->SetAttribute("overwritten", true);
    //     LegacyExperimentXMLBuilder::addValue(varElement, ovar.value);
    // }

    auto groups = group->getSubgroups();
    for (auto& g : groups) {
        addVars(timing, g);
    }

}

void addFiles(tinyxml2::XMLElement* timing, const std::filesystem::path& shotPath, const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& fileHolders)
{  
    for (auto& file : fileHolders) {
        if (file != 0) {
            std::filesystem::path absFilePath = file->getFilename();
            auto relativeFilePath = std::filesystem::relative(absFilePath, shotPath.parent_path());
            timing->InsertNewChildElement("file")->SetText(relativeFilePath.string().c_str());
        }
    }
}


void LegacyExperimentXMLBuilder::build()
{
    if (fullShotResult == 0) return;

    auto parseResult = fullShotResult->parseResult;
    auto shotResult = fullShotResult->shotResult;

    if (parseResult == 0) return;
    if (shotResult == 0) return;
    
    doc.Clear();

    auto e = doc.NewElement("experiment");
    doc.InsertFirstChild(e);

    auto title = e->InsertNewChildElement("title");
    std::filesystem::path shotPath = filename;
    title->SetText(shotPath.stem().c_str());

    auto date = e->InsertNewChildElement("date");
    date->SetText(shotResult->playTime.print().c_str());
    
    if (parseResult->pid.shotConfig.shotType == ShotType::Sequence) {

        //add relative path to sequence XML filename
        
        std::filesystem::path sequencePath = sequenceFilename;
        auto relativeSeqPath = std::filesystem::relative(sequencePath, shotPath.parent_path());

        auto series = e->InsertNewChildElement("series");
        series->SetText(relativeSeqPath.string().c_str());
    }

    //timing
    if (parseResult->baseEventGroup != 0) {
        auto timing = e->InsertNewChildElement("timing");
        if (parseResult->stackTraceResult != 0 && parseResult->stackTraceResult->stackTraceData != 0) {
            auto files = parseResult->stackTraceResult->stackTraceData->getTimingFiles();
            addFiles(timing, shotPath, files);
        }       
        addVars(timing, parseResult->baseEventGroup);
    }

    auto devices = e->InsertNewChildElement("devices");

    std::vector<STI::Device::DeviceID> deviceIDs;

    if (parseResult->parsedDevices != 0) {        
        parseResult->parsedDevices->getNodes(deviceIDs);
    }
    else {
        //use attribute map
    }

    //device data
    for (auto& id : deviceIDs) {
        auto device = devices->InsertNewChildElement("device");
        device->SetAttribute("devicename", id.getName().c_str());
        device->SetAttribute("ipaddress", id.getAddress().c_str());
        device->SetAttribute("module", id.getModule());

        auto attributes = device->InsertNewChildElement("attributes");

        auto attributesIt = shotResult->attributes.find(id);

        if (attributesIt != shotResult->attributes.end()) {
            addAttributes(attributes, attributesIt->second);
        }

        auto measurements = device->InsertNewChildElement("measurements");
        
        if (shotResult->measurements != 0) {
            auto measurementsIt = shotResult->measurements->find(id);

            if (measurementsIt != shotResult->measurements->end()) {
                addMeasurements(measurements, measurementsIt->second);
            }
        }

    }

    // doc.Print();

    
    // std::string filename {"test2.xml"};
    // std::fstream s {filename, s.binary | s.trunc | s.in | s.out};

    // tinyxml2::XMLPrinter printer( s );
    // doc.Print( &printer );
}

void LegacyExperimentXMLBuilder::write()
{
    doc.SaveFile(filename.c_str());
}
