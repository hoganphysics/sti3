
#ifndef STI_PYTHON_RAWSTACKTRACE_H
#define STI_PYTHON_RAWSTACKTRACE_H

#include <string>
#include <vector>


namespace STI
{
namespace Engine
{

struct RawStackFrame
{
    RawStackFrame();
    RawStackFrame(const std::string& file, unsigned line, const std::string& func);

	std::string file;
	unsigned line;
	std::string func;
};


class RawStackTrace
{
public:

    RawStackTrace();
    
    void appendFrame(const RawStackFrame& frame);
    void appendFrame(const std::string& file, unsigned line, const std::string& func);
    std::vector<RawStackFrame> getFrames() const;

private:

    std::vector<RawStackFrame> frames;
};


} //Engine
} //STI

#endif

