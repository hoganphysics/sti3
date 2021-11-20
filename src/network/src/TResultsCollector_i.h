#ifndef STI_TNETWORK_TRESULTSCOLLECTOR_I_H
#define STI_TNETWORK_TRESULTSCOLLECTOR_I_H

#include "fwd/ResultsCollector_fwd.h"
#include "deviceNet.h"

#include <memory>


namespace STI
{
namespace TNetwork
{


class TResultsCollector_i : public POA_STI::TNetwork::TResultsCollector
{
public:

	TResultsCollector_i(STI::Engine::ResultsCollector* resultsCollector);
	~TResultsCollector_i();

    TShotID* getShotID();
    void addEvents(const ::STI::TNetwork::TDeviceEventsSeq& parsedEvents);
    void addTimingFiles(const ::STI::TNetwork::TFileHolderSeq& files);
    ::CORBA::Boolean addMeasurements(const ::STI::TNetwork::TMeasurementSeq& measurements);
//    ::CORBA::Boolean addAttributes(const ::STI::TNetwork::TDeviceID& deviceID, const ::STI::TNetwork::TAttributeSeq& attributes);
    ::CORBA::Boolean addAttributes(const ::STI::TNetwork::TDeviceID& deviceID, const ::STI::TNetwork::TStringPairSeq& attributes);

    
private:

    STI::Engine::ResultsCollector* resultsCollector;

};


} //TNetwork
} //STI

#endif

