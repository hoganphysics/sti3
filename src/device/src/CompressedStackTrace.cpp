#include <sti/engine/CompressedStackTrace.h>

#include <string>
#include <sstream>
#include <vector>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::CompressedStackTrace;
using STI::Engine::CompressedStackFrame;


CompressedStackFrame::CompressedStackFrame()
{
}

CompressedStackFrame::CompressedStackFrame(unsigned file, unsigned line, unsigned func)
: file(file), line(line), func()
{
}

CompressedStackTrace::CompressedStackTrace()
{
}

CompressedStackTrace::~CompressedStackTrace()
{
}

void CompressedStackTrace::appendFrame(unsigned file, unsigned line, unsigned func)
{
    CompressedStackFrame frame;
    frame.file = file;
    frame.line = line;
    frame.func = func;
    
    frames.push_back(frame);
}

void CompressedStackTrace::appendFrame(const CompressedStackFrame& frame)
{
    frames.push_back(frame);
}

std::vector<CompressedStackFrame> CompressedStackTrace::getFrames() const
{
    return frames;
}

template<class Archive>
void CompressedStackFrame::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("file", file),
        cereal::make_nvp("line", line),
        cereal::make_nvp("func", func)
		);
}

template<class Archive>
void CompressedStackTrace::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("frames", frames)
		);
}

template void CompressedStackTrace::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void CompressedStackTrace::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

