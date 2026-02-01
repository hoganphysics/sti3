#include "TRefreshIndicator_i.h"

using STI::TNetwork::TRefreshIndicator_i;


TRefreshIndicator_i::TRefreshIndicator_i() 
	: updated(true)
{
}

TRefreshIndicator_i::~TRefreshIndicator_i()
{
}

///Reset and check status within the same mutex block to avoid any delay between check and reset
///which could cause a missed refresh()
void TRefreshIndicator_i::refresh()
{
	std::unique_lock<std::mutex> writelock(updateMutex);
	updated = true;
}

///Reset and check status within the same mutex block to avoid any delay between check and reset
///which could cause a missed refresh()
bool TRefreshIndicator_i::checkThenReset()
{
	std::unique_lock<std::mutex> writelock(updateMutex);
	bool updateOccurred = updated;
	updated = false;

	return updateOccurred;
}
