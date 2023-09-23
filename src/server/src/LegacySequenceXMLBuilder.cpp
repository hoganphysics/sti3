
#include "LegacySequenceXMLBuilder.h"
#include "LegacyExperimentXMLBuilder.h"

#include <sti/engine/ParsedVar.h>

#include <filesystem>

using STI::Engine::LegacySequenceXMLBuilder;
using STI::Engine::LegacyExperimentXMLBuilder;
using STI::Engine::SequenceMessageGrouper;
using STI::Engine::AddShotMessage;
using STI::Engine::ParsedVar;




LegacySequenceXMLBuilder::LegacySequenceXMLBuilder(const std::string& filename, const std::shared_ptr<SequenceResult>& sequenceResult)
: filename(filename), messageGrouper(this)
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

    auto script = e->InsertNewChildElement("script");
    //Script not available

    // sequenceResult->sequence->sequenceTable

    experiments = e->InsertNewChildElement("experiments");
    sequence = sequenceResult->sequence;

}

void addVars(tinyxml2::XMLElement* experiment, const std::set<ParsedVar>& vars, const std::string& shotFilename)
{
    for (auto& var : vars) {
        auto varElement = experiment->InsertNewChildElement("var");
        varElement->SetAttribute("name", var.name.c_str());
        LegacyExperimentXMLBuilder::addValue(varElement, var.value, shotFilename);
    }
}

void LegacySequenceXMLBuilder::addShot(const std::string& shotFilename, const SequenceEntryID& id)
{
    //Append shot filename to XML
    if (experiments == 0) return;
    if (sequence == 0) return;

    std::map<unsigned, SequenceEntry>::iterator it;
    bool closedSeq = false;

    if (sequence->type == SequenceType::Closed) {
        closedSeq = true;
        
        it = sequence->sequenceTable.find(id.seqIndex.index);
        if (it == sequence->sequenceTable.end()) return;
    }

    auto experiment = experiments->InsertNewChildElement("experiment");

    if (closedSeq) {
        addVars(experiment, it->second.overwritten, shotFilename);
    }

    std::filesystem::path sequencePath = filename;
    std::filesystem::path shotPath = shotFilename;
    auto relativeShotPath = std::filesystem::relative(shotPath, sequencePath.parent_path());

    auto file = experiment->InsertNewChildElement("file");
    file->SetText(relativeShotPath.c_str());

    // Message grouper is used to buffer addShot commands.
    // This limits the maximum rate that write() is called when shots are added rapidly.
    auto mess = std::make_shared<AddShotMessage>();
    messageGrouper.addMessage(mess);
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