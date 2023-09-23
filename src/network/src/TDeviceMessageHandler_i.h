#ifndef STI_TNETWORK_TDEVICEMESSAGEHANDLER_I_H
#define STI_TNETWORK_TDEVICEMESSAGEHANDLER_I_H

#include "LocalDeviceMessageHandler.h"

#include "generated/deviceNet.h"
#include "generated/orbTypes.h"
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>

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

	void disableRefreshIndicator();

private:

	std::shared_ptr<STI::Device::LocalDeviceMessageHandler> messageHandler;

	bool tRefreshIndicatorInstalled;

	std::unique_ptr<STI::TNetwork::TReferenceHolder<STI::TNetwork::TRefreshIndicator>> tRefreshIndicatorHolder;
	mutable std::mutex refreshMutex;
};

} //TNetwork
} //STI


#endif

