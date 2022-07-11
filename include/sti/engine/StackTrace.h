#ifndef STI_ENGINE_STACKTRACE_H
#define STI_ENGINE_STACKTRACE_H

#include <string>
#include <vector>

namespace STI
{
namespace Engine
{


struct StackFrame
{
	StackFrame();
	StackFrame(unsigned file, unsigned line, unsigned func);

	unsigned file;
	unsigned line;
	unsigned func;

	template<class Archive>
	void serialize(Archive& archive);
};


class StackTrace
{
public:

	StackTrace();
	~StackTrace();

	// std::string file() const;
	// long line() const;
	void appendFrame(unsigned file, unsigned line, unsigned func);
	void appendFrame(const StackFrame& frame);

	std::vector<StackFrame> getFrames() const;

	// std::string print(std::string indent = "       ") const;

	template<class Archive>
	void serialize(Archive& archive);

private:

	std::vector<StackFrame> frames;

};


} //Engine
} //STI

#endif
