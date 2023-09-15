#ifndef STI_ENGINE_LEGACYEXPERIMENTXMLBUILDER_H
#define STI_ENGINE_LEGACYEXPERIMENTXMLBUILDER_H

#include <sti/engine/FullShotResult.h>

#include <sti/utils/MixedValue.h>

#include <tinyxml2.h>

#include <string>
#include <memory>


namespace STI
{
namespace Engine
{

class LegacyExperimentXMLBuilder
{
public:

    LegacyExperimentXMLBuilder();
    LegacyExperimentXMLBuilder(const std::string& filename, const std::shared_ptr<FullShotResult>& fullShotResult);
    ~LegacyExperimentXMLBuilder();

    LegacyExperimentXMLBuilder& setShotResult(const std::string& filename, const std::shared_ptr<FullShotResult>& fullShotResult);
    LegacyExperimentXMLBuilder& addSequenceFilename(const std::string& seqFilename);

    void build();
    void write();

    // void buildXML(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult);
    
    static void addValue(tinyxml2::XMLElement* base, const STI::Utils::MixedValue& value, const std::string& shotFilename);

private:

    std::string filename;
    std::string sequenceFilename;
    std::shared_ptr<FullShotResult> fullShotResult;
    tinyxml2::XMLDocument doc;

};


} //Engine
} //STI

#endif

