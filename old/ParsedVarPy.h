#ifndef STI_PYTHON_PARSEDVARPY_H
#define STI_PYTHON_PARSEDVARPY_H


#include <sti/engine/ParsedVar.h>
#include "RawStackTrace.h"
#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEventGroup.h>

#include <pybind11/pybind11.h>

#include <string>
#include <memory>

namespace STI
{
namespace Python
{


class ParsedVarPy
{
public:

    ParsedVarPy();
    ParsedVarPy(const STI::Engine::ParsedVar& var, const std::shared_ptr<STI::Engine::ParseResult>& parseResult);

    std::string name() const;
    pybind11::object value() const;
    StackTracePy trace() const;
    STI::Engine::RawEventGroup scope() const;

    bool operator<(const ParsedVarPy& rhs) const;
    bool operator==(const ParsedVarPy& rhs) const;
    bool operator!=(const ParsedVarPy& rhs) const;

private:

    STI::Engine::ParsedVar var;
    std::shared_ptr<STI::Engine::ParseResult> parseResult;
};


} //Python
} //STI

#endif
