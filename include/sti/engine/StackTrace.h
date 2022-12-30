#ifndef STI_ENGINE_STACKTRACE_H
#define STI_ENGINE_STACKTRACE_H

#include <vector>


namespace STI
{
namespace Engine
{


class StackFrame
{
public:

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

	void appendFrame(unsigned file, unsigned line, unsigned func);
	void appendFrame(const StackFrame& frame);

	std::vector<StackFrame> getFrames() const;

	template<class Archive>
	void serialize(Archive& archive);

private:

	std::vector<StackFrame> frames;

};


} //Engine
} //STI

#endif
