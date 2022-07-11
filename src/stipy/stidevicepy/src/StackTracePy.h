
#ifndef STI_PYTHON_STACKTRACEPY_H
#define STI_PYTHON_STACKTRACEPY_H

// #include <sti/engine/StackTrace.h>

#include <string>
#include <vector>


namespace STI
{
namespace Python
{


struct StackFramePy
{
    StackFramePy();
    StackFramePy(const std::string& file, unsigned line, const std::string& func);

	std::string file;
	unsigned line;
	std::string func;
};


class StackTracePy
{
public:

    StackTracePy();
    
    void appendFrame(const StackFramePy& frame);
    void appendFrame(const std::string& file, unsigned line, const std::string& func);
    std::vector<StackFramePy> getFrames() const;

private:

    std::vector<StackFramePy> frames;
};


} //Python
} //STI

#endif

