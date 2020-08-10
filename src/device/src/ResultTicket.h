#ifndef STI_ENGINE_RESULTSTICKET_H
#define STI_ENGINE_RESULTSTICKET_H

#include "ShotID.h"
#include "fwd/Measurement_fwd.h"

#include <memory>

namespace STI
{
namespace Engine
{

class ResultTicket
{
public:

	ShotID getShotID();
	bool resultsReady();
	void notify();
	void getMeasurements(std::shared_ptr<MeasurementVector>);	//optional filter?
};


} //Engine
} //STI

#endif

