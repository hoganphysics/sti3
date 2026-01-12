#include "LegacyParseXMLBuilder.h"

#include <sti/engine/ParseResult.h>

#include <fstream>
#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::LegacyParseXMLBuilder;
using STI::Engine::ParseResult;

LegacyParseXMLBuilder::LegacyParseXMLBuilder() = default;

LegacyParseXMLBuilder::LegacyParseXMLBuilder(const std::string& filename, const std::shared_ptr<ParseResult>& parseResult)
: filename(filename), parseResult(parseResult)
{
}

LegacyParseXMLBuilder::~LegacyParseXMLBuilder() = default;

LegacyParseXMLBuilder& LegacyParseXMLBuilder::setParseResult(const std::string& filename, const std::shared_ptr<ParseResult>& parseResult)
{
    this->filename = filename;
    this->parseResult = parseResult;
    return *this;
}

void LegacyParseXMLBuilder::build()
{
    xmlBuffer.clear();

    if (parseResult == 0) return;

    std::ostringstream out;
    {
        cereal::XMLOutputArchive archive(out);
        archive(parseResult);
    }
    xmlBuffer = out.str();
}

void LegacyParseXMLBuilder::write()
{
    if (filename.empty()) return;

    if (xmlBuffer.empty()) {
        build();
    }

    if (xmlBuffer.empty()) return;

    std::ofstream file(filename);
    file << xmlBuffer;
}
