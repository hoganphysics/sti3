#ifndef STI_ENGINE_FULLSHOTRESULT_H
#define STI_ENGINE_FULLSHOTRESULT_H


#include <memory>

namespace STI
{
namespace Engine
{

class ParseResult;
class ShotResult;


class FullShotResult
{
public:

    std::shared_ptr<ParseResult> parseResult;
    std::shared_ptr<ShotResult> shotResult;

    template<class Archive>
    void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

