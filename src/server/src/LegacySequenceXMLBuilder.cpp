
#include "LegacySequenceXMLBuilder.h"
#include "LegacyExperimentXMLBuilder.h"

#include <sti/engine/ParsedVar.h>

#include <filesystem>
#include <string>

using STI::Engine::LegacySequenceXMLBuilder;
using STI::Engine::LegacyExperimentXMLBuilder;
using STI::Engine::SequenceMessageGrouper;
using STI::Engine::AddShotMessage;
using STI::Engine::ParsedVar;
using STI::Engine::EngineJobStatus;

namespace {

std::string sequenceTypeToString(const STI::Engine::SequenceType& type)
{
    switch (type) {
    case STI::Engine::SequenceType::Open:
        return "Open";
    case STI::Engine::SequenceType::Closed:
        return "Closed";
    default:
        return "Open";
    }
}

void setChildText(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const std::string& name, const std::string& value)
{
    if (parent == 0) return;

    auto child = parent->FirstChildElement(name.c_str());
    if (child == 0) {
        child = doc.NewElement(name.c_str());
        parent->InsertEndChild(child);
    }
    child->SetText(value.c_str());
}

void setChildText(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const std::string& name, unsigned value)
{
    if (parent == 0) return;

    auto child = parent->FirstChildElement(name.c_str());
    if (child == 0) {
        child = doc.NewElement(name.c_str());
        parent->InsertEndChild(child);
    }
    child->SetText(value);
}

void addSequenceIndex(tinyxml2::XMLElement* experiment, const STI::Engine::SequenceIndex& seqIndex)
{
    if (experiment == 0) return;

    auto sequenceIndex = experiment->InsertNewChildElement("sequenceindex");
    sequenceIndex->InsertNewChildElement("index")->SetText(seqIndex.index);
    sequenceIndex->InsertNewChildElement("repeat")->SetText(seqIndex.repeat);
}

std::filesystem::path safeRelativePath(const std::filesystem::path& target,
                                       const std::filesystem::path& base)
{
    if (base.empty()) {
        return target;
    }

    try {
        auto relative = std::filesystem::relative(target, base);
        if (!relative.empty()) {
            return relative;
        }
    }
    catch (const std::filesystem::filesystem_error&) {
    }

    auto lexical = target.lexically_relative(base);
    if (!lexical.empty()) {
        return lexical;
    }

    return target;
}

} // namespace




LegacySequenceXMLBuilder::LegacySequenceXMLBuilder(const std::string& filename, const std::shared_ptr<SequenceResult>& sequenceResult)
: filename(filename), messageGrouper(this), sequenceInfo(0), experiments(0), currentEntryCount(0)
{
    build(filename, sequenceResult);
}

LegacySequenceXMLBuilder::~LegacySequenceXMLBuilder()
{
}

std::string LegacySequenceXMLBuilder::getFilename() const
{
    return filename;
}

void LegacySequenceXMLBuilder::build(const std::string& filename, const std::shared_ptr<SequenceResult>& sequenceResult)
{
    //XML
    if (sequenceResult == 0) return;

    doc.Clear();

    auto e = doc.NewElement("series");
    doc.InsertFirstChild(e);

    auto title = e->InsertNewChildElement("title");
    std::filesystem::path xmlPath = filename;
    title->SetText(xmlPath.stem().c_str());

    auto date = e->InsertNewChildElement("date");
    date->SetText(sequenceResult->seqid.timestamp.print().c_str());

    sequence = sequenceResult->sequence;
    currentEntryCount = 0;
    experimentEntries.clear();

    sequenceInfo = e->InsertNewChildElement("sequence");
    updateSequenceInfo();

    auto script = e->InsertNewChildElement("script");
    //Script not available

    // sequenceResult->sequence->sequenceTable

    experiments = e->InsertNewChildElement("experiments");

}

void addVars(tinyxml2::XMLElement* experiment, const std::set<ParsedVar>& vars, const std::string& shotFilename)
{
    for (auto& var : vars) {
        auto varElement = experiment->InsertNewChildElement("var");
        varElement->SetAttribute("name", var.name.c_str());
        LegacyExperimentXMLBuilder::addValue(varElement, var.value, shotFilename);
    }
}

void LegacySequenceXMLBuilder::addShot(const std::string& shotFilename, const std::string& parseFilename,
                                       const SequenceEntryID& id, const EngineJobStatus& shotStatus)
{
    addEntry(shotFilename, parseFilename, id, shotStatus);
}

void LegacySequenceXMLBuilder::addParseResult(const std::string& parseFilename, const SequenceEntryID& id, const EngineJobStatus& parseStatus)
{
    addEntry("", parseFilename, id, parseStatus);
}

void LegacySequenceXMLBuilder::addEntry(const std::string& shotFilename, const std::string& parseFilename,
                                        const SequenceEntryID& id, const EngineJobStatus& status)
{
    if (experiments == 0) return;
    if (sequence == 0) return;

    std::map<SequenceIndex, SequenceEntry>::iterator it;
    bool closedSeq = false;

    if (sequence->type == SequenceType::Closed) {
        closedSeq = true;
        
        it = sequence->sequenceTable.find(id.seqIndex);
        if (it == sequence->sequenceTable.end()) return;
    }

    tinyxml2::XMLElement* experiment = 0;
    auto existingEntry = experimentEntries.find(id.seqIndex);
    if (existingEntry == experimentEntries.end()) {
        experiment = experiments->InsertNewChildElement("experiment");
        experimentEntries[id.seqIndex] = experiment;

        addSequenceIndex(experiment, id.seqIndex);

        if (closedSeq) {
            const auto& valueFilename = shotFilename.empty() ? parseFilename : shotFilename;
            addVars(experiment, it->second.overwritten, valueFilename);
        }

        currentEntryCount = static_cast<unsigned>(experimentEntries.size());
    }
    else {
        experiment = existingEntry->second;
    }

    setChildText(doc, experiment, "status", EngineJobStatusToString(status));

    std::filesystem::path sequencePath = filename;
    if (!parseFilename.empty()) {
        std::filesystem::path parsePath = parseFilename;
        auto relativeParsePath = safeRelativePath(parsePath, sequencePath.parent_path());

        auto parse = experiment->FirstChildElement("parse");
        if (parse == 0) {
            parse = experiment->InsertNewChildElement("parse");
        }
        setChildText(doc, parse, "file", relativeParsePath.string());
    }

    if (!shotFilename.empty()) {
        std::filesystem::path shotPath = shotFilename;
        auto relativeShotPath = safeRelativePath(shotPath, sequencePath.parent_path());
        setChildText(doc, experiment, "file", relativeShotPath.string());
    }
    else {
        auto shotFile = experiment->FirstChildElement("file");
        if (shotFile != 0) {
            experiment->DeleteChild(shotFile);
        }
    }

    updateSequenceInfo();

    // Message grouper is used to buffer addShot commands.
    // This limits the maximum rate that write() is called when shots are added rapidly.
    auto mess = std::make_shared<AddShotMessage>();
    messageGrouper.addMessage(mess);
}

void LegacySequenceXMLBuilder::updateSequenceInfo()
{
    if (sequenceInfo == 0 || sequence == 0) return;

    setChildText(doc, sequenceInfo, "type", sequenceTypeToString(sequence->type));
    setChildText(doc, sequenceInfo, "current", currentEntryCount);

    auto expected = sequenceInfo->FirstChildElement("expected");
    if (sequence->type == SequenceType::Closed) {
        setChildText(doc, sequenceInfo, "expected", static_cast<unsigned>(sequence->sequenceTable.size()));
    }
    else if (expected != 0) {
        sequenceInfo->DeleteChild(expected);
    }
}

void LegacySequenceXMLBuilder::write()
{
    doc.SaveFile(filename.c_str());
}


void SequenceMessageGrouper::dispatchMessage(const std::shared_ptr<AddShotMessage>& mess)
{
    if (builder != 0) {
        builder->write();
    }
}
