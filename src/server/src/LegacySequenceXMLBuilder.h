#ifndef STI_ENGINE_LEGACYSEQUENCEXMLBUILDER_H
#define STI_ENGINE_LEGACYSEQUENCEXMLBUILDER_H

#include <sti/engine/SequenceResult.h>
#include <sti/engine/SequenceID.h>

#include <sti/device/MessageGrouper.h>

#include <tinyxml2.h>

#include <map>
#include <string>
#include <memory>


namespace STI
{
namespace Engine
{

class AddShotMessage;
class SequenceMessageGrouper;
class LegacySequenceXMLBuilder;


class SequenceMessageGrouper : public STI::Device::MessageGrouper<AddShotMessage>
{
public:

    SequenceMessageGrouper(LegacySequenceXMLBuilder* builder) : builder(builder) {}

    void dispatchMessage(const std::shared_ptr<AddShotMessage>& mess);

private:
    LegacySequenceXMLBuilder* builder;
};




class LegacySequenceXMLBuilder
{
public:

    LegacySequenceXMLBuilder(const std::string& filename, const std::shared_ptr<SequenceResult>& sequenceResult);
    ~LegacySequenceXMLBuilder();

    void build(const std::string& filename, const std::shared_ptr<SequenceResult>& sequenceResult);
    void addShot(const std::string& shotFilename, const std::string& parseFilename,
        const SequenceEntryID& id, const EngineJobStatus& shotStatus);
    void addParseResult(const std::string& parseFilename, const SequenceEntryID& id, const EngineJobStatus& parseStatus);
    void write();

    std::string getFilename() const;

private:

    SequenceMessageGrouper messageGrouper;

    std::string filename;
    std::shared_ptr<Sequence> sequence;

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* sequenceInfo;
    tinyxml2::XMLElement* experiments;
    std::map<SequenceIndex, tinyxml2::XMLElement*> experimentEntries;

    unsigned currentEntryCount;

    void addEntry(const std::string& shotFilename, const std::string& parseFilename,
        const SequenceEntryID& id, const EngineJobStatus& status);
    void updateSequenceInfo();
};




class AddShotMessage : public STI::Device::GroupableMessage<AddShotMessage>
{
public:
    AddShotMessage() {}
    bool appendMessage(const AddShotMessage& mess) { return true; }
    bool groupable() const { return true; }
    AddShotMessage& get() {return *this; }
};


} //Engine
} //STI

#endif
