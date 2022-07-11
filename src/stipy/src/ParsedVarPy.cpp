
#include "ParsedVarPy.h"
#include "MixedValuePy.h"

using STI::Python::ParsedVarPy;
using STI::Python::StackTracePy;
using STI::Engine::ParsedVar;
using STI::Engine::RawEventGroup;
using STI::Engine::StackTrace;
using STI::Python::MixedValuePy;


ParsedVarPy::ParsedVarPy()
{
}

ParsedVarPy::ParsedVarPy(const STI::Engine::ParsedVar& var, const std::shared_ptr<STI::Engine::ParseResult>& parseResult)
: var(var), parseResult(parseResult)
{
}

std::string ParsedVarPy::name() const
{
    return var.name;
}

pybind11::object ParsedVarPy::value() const
{
    MixedValuePy pyVal;
    pyVal.setValue(var.value);
    return pyVal.getValue_py();
}

StackTracePy ParsedVarPy::trace() const
{
    StackTracePy pyTrace;

    for (auto& f : var.trace.getFrames()) {

        if (parseResult != 0 
                && parseResult->timingFiles.size() < f.file 
                && parseResult->timingFiles.at(f.file) != 0 
                && parseResult->functionNames.size() < f.func) 
        {    
            pyTrace.appendFrame(
                parseResult->timingFiles.at(f.file)->getFilename(),
                f.line,
                parseResult->functionNames.at(f.func)
                );
        }
    }
    
    return pyTrace;
}

RawEventGroup ParsedVarPy::scope() const
{
    return var.scope;
}


bool ParsedVarPy::operator<(const ParsedVarPy& rhs) const
{
    return (var < rhs.var);
}

bool ParsedVarPy::operator==(const ParsedVarPy& rhs) const
{
    return (var == rhs.var);
}

bool ParsedVarPy::operator!=(const ParsedVarPy& rhs) const
{
    return (var != rhs.var);
}
