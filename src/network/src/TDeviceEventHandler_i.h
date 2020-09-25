#ifndef STI_TNETWORK_TDEVICEEVENTHANDLER_I_H
#define STI_TNETWORK_TDEVICEEVENTHANDLER_I_H

#include "LocalDeviceEventHandler.h"

#include "deviceNet.h"
#include "orbTypes.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TDeviceEventHandler_i : public POA_STI::TNetwork::TDeviceEventHandler
{
public:

	TDeviceEventHandler_i(const std::shared_ptr<STI::Device::LocalDeviceEventHandler>& handler);
	~TDeviceEventHandler_i();

	void addEvent(const ::STI::TNetwork::TAnyEvent& evt);
	void clearEvents();
	TDeviceEventTypeSeq* listenersTypes();
	void setRefreshIndicator(::STI::TNetwork::TRefreshIndicator_ptr refresher);

	void refresh();

private:

	std::shared_ptr<STI::Device::LocalDeviceEventHandler> eventHandler;

	::STI::TNetwork::TRefreshIndicator_var tRefreshIndicator;
};

} //TNetwork
} //STI


#endif

