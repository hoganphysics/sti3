#include "LegacyExperimentXMLReader.h"

#include <sti/engine/EnginePlayingMessage.h>
#include <sti/engine/Measurement.h>
#include <sti/utils/Image.h>
#include <sti/utils/TimeStamp.h>
#include <sti/utils/utils.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


using STI::Engine::LegacyExperimentXMLReader;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::Measurement;
using STI::Engine::MeasurementMap;
using STI::Engine::MeasurementVector;
using STI::Engine::EnginePlayingMessage;
using STI::Engine::PlayingMessageType;
using STI::Device::DeviceID;
using STI::Utils::MixedValue;
using STI::Utils::FileID;
using STI::Utils::TimeStamp;
using STI::Utils::Image;

namespace {

std::string toLowerCopy(const std::string& input)
{
    std::string lower = input;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

PlayingMessageType messageTypeFromString(const std::string& typeString)
{
    auto lowered = toLowerCopy(typeString);
    if (lowered == "warning") {
        return PlayingMessageType::Warning;
    }
    if (lowered == "information" || lowered == "info") {
        return PlayingMessageType::Information;
    }
    return PlayingMessageType::Error;
}

bool parseBoolText(const char* text, bool& value)
{
    if (text == nullptr) return false;

    auto trimmed = STI::Utils::trim(text);
    auto lowered = toLowerCopy(trimmed);

    if (lowered == "true" || lowered == "1") {
        value = true;
        return true;
    }
    if (lowered == "false" || lowered == "0") {
        value = false;
        return true;
    }
    return false;
}

bool parsePrintedTimestamp(const std::string& text, TimeStamp& output)
{
    auto trimmed = STI::Utils::trim(text);
    if (trimmed.empty()) {
        return false;
    }

    if (trimmed.find('|') != std::string::npos) {
        output = TimeStamp::fromString(trimmed);
        return true;
    }

    auto openParen = trimmed.find('(');
    auto closeParen = trimmed.find(')', openParen == std::string::npos ? 0 : openParen + 1);
    if (openParen == std::string::npos || closeParen == std::string::npos) {
        return false;
    }

    std::string date = trimmed.substr(openParen + 1, closeParen - openParen - 1);
    auto timePart = STI::Utils::trim(trimmed.substr(closeParen + 1));

    if (date.empty()) {
        return false;
    }

    std::vector<std::string> tokens;
    std::istringstream stream(timePart);
    for (std::string token; stream >> token;) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        output = TimeStamp::fromString(date);
        return true;
    }

    std::string hhmmss = tokens.at(0);
    std::string mmm = "0";
    std::string uuu = "0";
    std::string nnn = "0";

    auto dotPos = hhmmss.find('.');
    if (dotPos != std::string::npos) {
        mmm = hhmmss.substr(dotPos + 1);
        hhmmss = hhmmss.substr(0, dotPos);
    }
    if (tokens.size() > 1) {
        uuu = tokens.at(1);
    }
    if (tokens.size() > 2) {
        nnn = tokens.at(2);
    }

    std::stringstream normalized;
    normalized << date << "|" << hhmmss << "." << mmm << "." << uuu << "." << nnn;

    output = TimeStamp::fromString(normalized.str());
    return true;
}

std::filesystem::path resolvePath(const std::filesystem::path& base, const std::string& candidate)
{
    std::filesystem::path path(candidate);
    if (path.is_relative()) {
        return base / path;
    }
    return path;
}

bool parseFileElement(const tinyxml2::XMLElement* element,
                      const std::filesystem::path& shotPath,
                      MixedValue& value)
{
    const tinyxml2::XMLElement* filenameElement = element->FirstChildElement("filename");
    const char* filenameText = filenameElement != nullptr ? filenameElement->GetText() : element->GetText();
    if (filenameText == nullptr) {
        value.setValue();
        return false;
    }

    auto resolved = resolvePath(shotPath.parent_path(), filenameText);

    FileID fileID;
    fileID.path = resolved.parent_path().string();
    fileID.filename = resolved.filename().string();

    bool isImage = (element->FirstChildElement("md5hash") != nullptr);
    if (isImage) {
        auto image = std::make_shared<Image>(fileID);
        value.setValue(image);
    }
    else {
        value.setValue(fileID);
    }

    return true;
}

bool parseDelimitedVector(const tinyxml2::XMLElement* element,
                          MixedValue& value)
{
    const char* text = element->GetText();
    std::vector<int> values;

    std::string delimiter = ",";
    if (const char* delimiterAttr = element->Attribute("delimiter")) {
        delimiter = delimiterAttr;
    }

    if (text != nullptr) {
        std::vector<std::string> tokens;
        STI::Utils::splitString(text, delimiter, tokens);
        for (auto& token : tokens) {
            auto trimmed = STI::Utils::trim(token);
            if (trimmed.empty()) {
                continue;
            }
            int parsed = 0;
            if (STI::Utils::stringToValue(trimmed, parsed)) {
                values.push_back(parsed);
            }
        }
    }

    value.setValue(values);
    return true;
}

bool parseDelimitedVectorToGraphPath(const tinyxml2::XMLElement* element,
                                     STI::Utils::GraphPathLabel& graphPath)
{
    graphPath.clear();
    if (element == nullptr) {
        return false;
    }

    const char* text = element->GetText();
    std::string delimiter = ",";
    if (const char* delimiterAttr = element->Attribute("delimiter")) {
        delimiter = delimiterAttr;
    }

    if (text == nullptr) {
        return false;
    }

    std::vector<std::string> tokens;
    STI::Utils::splitString(text, delimiter, tokens);
    for (auto& token : tokens) {
        auto trimmed = STI::Utils::trim(token);
        if (trimmed.empty()) {
            continue;
        }
        int parsed = 0;
        if (STI::Utils::stringToValue(trimmed, parsed) && parsed >= 0) {
            graphPath.push_back(static_cast<unsigned>(parsed));
        }
    }

    return !graphPath.empty();
}

bool parseMixedValueElement(const tinyxml2::XMLElement* element,
                            const std::filesystem::path& shotPath,
                            MixedValue& value)
{
    if (element == nullptr || element->Name() == nullptr) {
        value.setValue();
        return false;
    }

    std::string elementName = element->Name();

    if (elementName == "bool") {
        bool parsed = false;
        if (parseBoolText(element->GetText(), parsed)) {
            value.setValue(parsed);
            return true;
        }
        value.setValue();
        return false;
    }

    if (elementName == "int") {
        int parsed = 0;
        if (element->GetText() != nullptr && STI::Utils::stringToValue(element->GetText(), parsed)) {
            value.setValue(parsed);
            return true;
        }
        value.setValue();
        return false;
    }

    if (elementName == "double") {
        double parsed = 0.0;
        if (element->GetText() != nullptr && STI::Utils::stringToValue(element->GetText(), parsed)) {
            value.setValue(parsed);
            return true;
        }
        value.setValue();
        return false;
    }

    if (elementName == "string") {
        const char* text = element->GetText();
        value.setValue(text != nullptr ? std::string(text) : std::string());
        return true;
    }

    if (elementName == "file") {
        return parseFileElement(element, shotPath, value);
    }

    if (elementName == "vector") {
        bool hasValues = false;
        MixedValue vectorValue;

        for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
            MixedValue childValue;
            if (parseMixedValueElement(child, shotPath, childValue)) {
                vectorValue.addValue(childValue);
                hasValues = true;
            }
        }

        if (!hasValues) {
            vectorValue.setValue(std::vector<std::string>());
        }

        value = vectorValue;
        return true;
    }

    if (elementName == "delimitedvector") {
        return parseDelimitedVector(element, value);
    }

    value.setValue();
    return false;
}

bool parseMixedValueFromContainer(const tinyxml2::XMLElement* container,
                                  const std::filesystem::path& shotPath,
                                  MixedValue& value)
{
    if (container == nullptr) {
        value.setValue();
        return false;
    }

    for (auto child = container->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
        if (child->Name() == nullptr) {
            continue;
        }

        std::string childName = child->Name();
        if (childName == "graphpath") {
            continue;
        }

        return parseMixedValueElement(child, shotPath, value);
    }

    value.setValue();
    return false;
}

} // namespace

LegacyExperimentXMLReader::LegacyExperimentXMLReader()
{
}

LegacyExperimentXMLReader::LegacyExperimentXMLReader(const std::string& filename)
: filename(filename)
{
}

LegacyExperimentXMLReader::~LegacyExperimentXMLReader()
{
}

LegacyExperimentXMLReader& LegacyExperimentXMLReader::setFilename(const std::string& filenameIn)
{
    filename = filenameIn;
    return *this;
}

bool LegacyExperimentXMLReader::readShotResult(const ShotID& shotID, std::shared_ptr<ShotResult>& shotResult)
{
    shotResult.reset();

    if (filename.empty()) {
        return false;
    }

    doc.Clear();
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        return false;
    }

    auto experiment = doc.FirstChildElement("experiment");
    if (experiment == nullptr) {
        return false;
    }

    auto loadedResult = std::make_shared<ShotResult>();
    loadedResult->sid = shotID;

    loadedResult->playTime = shotID.submissionTime;

    if (auto dateElement = experiment->FirstChildElement("date")) {
        if (const char* dateText = dateElement->GetText()) {
            TimeStamp parsed;
            if (parsePrintedTimestamp(dateText, parsed)) {
                loadedResult->playTime = parsed;
            }
        }
    }

    loadedResult->measurements = std::make_shared<MeasurementMap>();

    std::filesystem::path shotPath(filename);

    if (auto messagesElement = experiment->FirstChildElement("messages")) {
        for (auto messageElement = messagesElement->FirstChildElement("message");
             messageElement != nullptr;
             messageElement = messageElement->NextSiblingElement("message")) {

            std::string typeText;
            if (const char* typeAttr = messageElement->Attribute("type")) {
                typeText = typeAttr;
            }

            std::string sourceText;
            if (const char* sourceAttr = messageElement->Attribute("source")) {
                sourceText = sourceAttr;
            }

            unsigned id = 0;
            messageElement->QueryUnsignedAttribute("id", &id);

            std::string nameText;
            if (const char* nameAttr = messageElement->Attribute("name")) {
                nameText = nameAttr;
            }

            EnginePlayingMessage message(messageTypeFromString(typeText), id, nameText);

            if (!sourceText.empty()) {
                message.setSourceID(DeviceID(sourceText));
            }

            if (const char* text = messageElement->GetText()) {
                message.appendMessage(text);
            }

            loadedResult->messages.push_back(message);
        }
    }

    if (auto devicesElement = experiment->FirstChildElement("devices")) {
        for (auto deviceElement = devicesElement->FirstChildElement("device");
             deviceElement != nullptr;
             deviceElement = deviceElement->NextSiblingElement("device")) {

            std::string deviceName;
            std::string deviceAddress;
            unsigned moduleValue = 0;

            if (const char* nameAttr = deviceElement->Attribute("devicename")) {
                deviceName = nameAttr;
            }
            if (const char* addressAttr = deviceElement->Attribute("ipaddress")) {
                deviceAddress = addressAttr;
            }
            deviceElement->QueryUnsignedAttribute("module", &moduleValue);

            DeviceID deviceID(deviceName, deviceAddress, static_cast<unsigned short>(moduleValue));

            auto& attributes = loadedResult->attributes[deviceID];
            if (auto attributesElement = deviceElement->FirstChildElement("attributes")) {
                for (auto attributeElement = attributesElement->FirstChildElement("attribute");
                     attributeElement != nullptr;
                     attributeElement = attributeElement->NextSiblingElement("attribute")) {

                    std::string key;
                    std::string value;

                    if (const char* keyAttr = attributeElement->Attribute("key")) {
                        key = keyAttr;
                    }
                    if (const char* valueAttr = attributeElement->Attribute("value")) {
                        value = valueAttr;
                    }

                    if (!key.empty()) {
                        attributes[key] = value;
                    }
                }
            }

            MeasurementVector measurements;
            if (auto measurementsElement = deviceElement->FirstChildElement("measurements")) {
                for (auto measurementElement = measurementsElement->FirstChildElement("measurement");
                     measurementElement != nullptr;
                     measurementElement = measurementElement->NextSiblingElement("measurement")) {

                    double time = 0.0;
                    unsigned channel = 0;
                    measurementElement->QueryDoubleAttribute("time", &time);
                    measurementElement->QueryUnsignedAttribute("channel", &channel);

                    std::string groupName;
                    if (const char* groupAttr = measurementElement->Attribute("group")) {
                        groupName = groupAttr;
                    }

                    STI::Utils::GraphPathLabel graphPath;
                    if (auto graphPathElement = measurementElement->FirstChildElement("graphpath")) {
                        if (auto delimited = graphPathElement->FirstChildElement("delimitedvector")) {
                            parseDelimitedVectorToGraphPath(delimited, graphPath);
                        }
                        else {
                            parseDelimitedVectorToGraphPath(graphPathElement, graphPath);
                        }
                    }

                    auto measurement = std::make_shared<Measurement>(time,
                                                                    static_cast<unsigned short>(channel),
                                                                    deviceID,
                                                                    graphPath,
                                                                    groupName);

                    MixedValue data;
                    if (parseMixedValueFromContainer(measurementElement, shotPath, data)) {
                        measurement->setMeasurementResult(std::move(data));
                    }

                    measurements.push_back(measurement);
                }
            }

            (*loadedResult->measurements)[deviceID] = std::move(measurements);
        }
    }

    shotResult = loadedResult;
    return (shotResult != nullptr);
}
