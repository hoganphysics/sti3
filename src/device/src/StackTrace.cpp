#include <sti/engine/StackTrace.h>

#include <string>
#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::StackTrace;
using STI::Engine::StackFrame;


StackFrame::StackFrame()
{
}

StackFrame::StackFrame(unsigned file, unsigned line, unsigned func)
: file(file), line(line), func()
{
}

StackTrace::StackTrace()
{
}

StackTrace::~StackTrace()
{
}

void StackTrace::appendFrame(unsigned file, unsigned line, unsigned func)
{
    StackFrame frame;
    frame.file = file;
    frame.line = line;
    frame.func = func;
    
    frames.push_back(frame);
}

void StackTrace::appendFrame(const StackFrame& frame)
{
    frames.push_back(frame);
}

std::vector<StackFrame> StackTrace::getFrames() const
{
    return frames;
}

template<class Archive>
void StackFrame::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("file", file),
        cereal::make_nvp("line", line),
        cereal::make_nvp("func", func)
		);
}


template<class Archive>
void StackTrace::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("frames", frames)
		);
}

template void StackTrace::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void StackTrace::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

