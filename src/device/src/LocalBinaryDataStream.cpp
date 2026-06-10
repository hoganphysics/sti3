
#include "LocalBinaryDataStream.h"

#include <sti/utils/BinaryData.h>



using STI::Utils::LocalBinaryDataStream;
using STI::Utils::LocalBinaryDataStreamTarget;


//////////////////////  LocalBinaryDataStreamTarget  ///////////////////////


LocalBinaryDataStreamTarget::LocalBinaryDataStreamTarget(const std::shared_ptr<BinaryData>& target)
: target(target)
{
}

LocalBinaryDataStreamTarget::~LocalBinaryDataStreamTarget()
{
}

void LocalBinaryDataStreamTarget::start()
{
	chunks.clear();
}

void LocalBinaryDataStreamTarget::writeNext(const std::shared_ptr<BinaryData>& data)
{
	chunks.push_back(data);
}

void LocalBinaryDataStreamTarget::stop()
{
	target->merge(chunks);
}


//////////////////////  LocalBinaryDataStream  ///////////////////////


LocalBinaryDataStream::LocalBinaryDataStream(BinaryData* data, size_t chunkSize)
: data(data), chunkSize(chunkSize)
{
}

LocalBinaryDataStream::~LocalBinaryDataStream()
{
}

void LocalBinaryDataStream::transfer(const std::shared_ptr<BinaryDataStreamTarget>& target)
{
	if (data == 0) return;
	if (target == 0) return;

	if (data->hasLocalData()) {
		std::vector<std::shared_ptr<BinaryData>> chunks;
		data->split(chunks, chunkSize);

		target->start();

		for (auto& chunk : chunks) {
			target->writeNext(chunk);
		}

		target->stop();
	}
	else {
		data->transferTo(target);
	}
}
