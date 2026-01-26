#ifndef STI_ENGINE_LEGACYEXPERIMENTXMLREADER_H
#define STI_ENGINE_LEGACYEXPERIMENTXMLREADER_H

#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>

#include <tinyxml2.h>

#include <memory>
#include <string>


namespace STI
{
namespace Engine
{

class LegacyExperimentXMLReader
{
public:

    LegacyExperimentXMLReader();
    explicit LegacyExperimentXMLReader(const std::string& filename);
    ~LegacyExperimentXMLReader();

    LegacyExperimentXMLReader& setFilename(const std::string& filename);

    bool readShotResult(const ShotID& shotID, std::shared_ptr<ShotResult>& shotResult);

private:

    std::string filename;
    tinyxml2::XMLDocument doc;
};


} //Engine
} //STI

#endif
