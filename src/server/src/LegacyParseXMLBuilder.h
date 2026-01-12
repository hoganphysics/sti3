#ifndef STI_ENGINE_LEGACYPARSEXMLBUILDER_H
#define STI_ENGINE_LEGACYPARSEXMLBUILDER_H

#include <memory>
#include <string>

namespace STI
{
namespace Engine
{

class ParseResult;

class LegacyParseXMLBuilder
{
public:

    LegacyParseXMLBuilder();
    LegacyParseXMLBuilder(const std::string& filename, const std::shared_ptr<ParseResult>& parseResult);
    ~LegacyParseXMLBuilder();

    LegacyParseXMLBuilder& setParseResult(const std::string& filename, const std::shared_ptr<ParseResult>& parseResult);

    void build();
    void write();

private:

    std::string filename;
    std::shared_ptr<ParseResult> parseResult;
    std::string xmlBuffer;
};


} //Engine
} //STI

#endif
