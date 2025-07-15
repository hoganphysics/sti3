#ifndef STI_ENGINE_COMPRESSEDSTACKTRACE_H
#define STI_ENGINE_COMPRESSEDSTACKTRACE_H

#include <vector>


namespace STI
{
namespace Engine
{


class CompressedStackFrame
{
public:

	CompressedStackFrame();
	CompressedStackFrame(unsigned file, unsigned line, unsigned func);

	unsigned file;
	unsigned line;
	unsigned func;

	template<class Archive>
	void serialize(Archive& archive);
};


class CompressedStackTrace
{
public:

	CompressedStackTrace();
	~CompressedStackTrace();

	void appendFrame(unsigned file, unsigned line, unsigned func);
	void appendFrame(const CompressedStackFrame& frame);

	std::vector<CompressedStackFrame> getFrames() const;

	template<class Archive>
	void serialize(Archive& archive);

private:

	std::vector<CompressedStackFrame> frames;

};


} //Engine
} //STI

#endif
