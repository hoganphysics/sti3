
#ifndef STI_NETWORK_CONVERT_TASK_H
#define STI_NETWORK_CONVERT_TASK_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include "generated/orbTypes.h"
#include "generated/tasks.h"


#include <sti/utils/Task.h>

#include <memory>
#include <vector>


namespace STI
{

namespace Device
{


} //Device



template<>
std::shared_ptr<Utils::Task> Network::convert<TNetwork::TTask, std::shared_ptr<Utils::Task>>(const TNetwork::TTask& tTask);
template<>
TNetwork::TTask Network::convert<std::shared_ptr<Utils::Task>, TNetwork::TTask>(const std::shared_ptr<Utils::Task>& task);


template<>
bool Network::convert<TNetwork::TTask, std::shared_ptr<Utils::Task>>(const TNetwork::TTask& tTask, std::shared_ptr<Utils::Task>& task);
template<>
bool Network::convert<std::shared_ptr<Utils::Task>, TNetwork::TTask>(const std::shared_ptr<Utils::Task>& task, TNetwork::TTask& tTask);


//TaskStatus
template<>
Utils::TaskStatus Network::convert<TNetwork::TTaskStatus, Utils::TaskStatus>(const TNetwork::TTaskStatus& tStatus);
template<>
TNetwork::TTaskStatus Network::convert<Utils::TaskStatus, TNetwork::TTaskStatus>(const Utils::TaskStatus& status);


} //STI

#endif

