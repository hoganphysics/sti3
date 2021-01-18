#ifndef STI_TNETWORK_TDEVICEMESSAGEHANDLER_I_H
#define STI_TNETWORK_TDEVICEMESSAGEHANDLER_I_H

#include "LocalDeviceMessageHandler.h"

#include "deviceNet.h"
#include "orbTypes.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TDeviceMessageHandler_i : public POA_STI::TNetwork::TDeviceMessageHandler
{
public:

	TDeviceMessageHandler_i(const std::shared_ptr<STI::Device::LocalDeviceMessageHandler>& handler);
	~TDeviceMessageHandler_i();

	void addMessage(const ::STI::TNetwork::TAnyMessage& mess);
	void clearMessages();
	TDeviceMessageTypeSeq* listenersTypes();
	void setRefreshIndicator(::STI::TNetwork::TRefreshIndicator_ptr refresher);

	void refresh();

private:

	std::shared_ptr<STI::Device::LocalDeviceMessageHandler> messageHandler;

	::STI::TNetwork::TRefreshIndicator_var tRefreshIndicator;
	bool tRefreshIndicatorInstalled;
};

} //TNetwork
} //STI


#endif

