
#ifndef STI_ENGINE_SHOTREPOSITORY_H
#define STI_ENGINE_SHOTREPOSITORY_H

#include "fwd/Measurement_fwd.h"

#include <memory>

namespace STI
{
namespace Engine
{

class ShotID;
class ParseTicket;

class ShotRepository
{
public:

    virtual bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements) = 0;
    virtual bool getParseTicket(const ShotID& sid, std::shared_ptr<ParseTicket>& parseTicket) = 0;    //associated ParseTicket has events, etc.

};



} //Engine
} //STI

#endif

