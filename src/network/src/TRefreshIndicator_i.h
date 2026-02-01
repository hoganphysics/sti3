#ifndef STI_TNETWORK_TREFRESHINDICATOR_I_H
#define STI_TNETWORK_TREFRESHINDICATOR_I_H

#include "generated/deviceNet.h"
#include <mutex>

namespace STI
{
namespace TNetwork
{

class TRefreshIndicator_i : public POA_STI::TNetwork::TRefreshIndicator,
							public PortableServer::RefCountServantBase
{
public:

	TRefreshIndicator_i();
	~TRefreshIndicator_i();
	
	void refresh();

	///Reset and check status within the same mutex block to avoid any delay between check and reset
	///which could cause a missed refresh()
	bool checkThenReset();

private:

	bool updated;

	mutable std::mutex updateMutex;
};

} //TNetwork
} //STI


#endif

