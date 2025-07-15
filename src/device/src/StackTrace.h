
#ifndef STI_PYTHON_STACKTRACE_H
#define STI_PYTHON_STACKTRACE_H

#include <string>
#include <vector>


namespace STI
{
namespace Engine
{

struct StackFrame
{
    StackFrame();
    StackFrame(const std::string& file, unsigned line, const std::string& func);

	std::string file;
	unsigned line;
	std::string func;
};


class StackTrace
{
public:

    StackTrace();
    
    void appendFrame(const StackFrame& frame);
    void appendFrame(const std::string& file, unsigned line, const std::string& func);
    std::vector<StackFrame> getFrames() const;

private:

    std::vector<StackFrame> frames;
};


} //Engine
} //STI

#endif

