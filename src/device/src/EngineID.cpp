
#include <sti/engine/EngineID.h>

using STI::Engine::EngineID;


EngineID::EngineID() : engineNumber(0)
{
}

EngineID::EngineID(int number) : engineNumber(number)
{
}

bool EngineID::operator<(const EngineID& rhs) const
{
    return(engineNumber < rhs.engineNumber);
}

bool EngineID::operator==(const EngineID& rhs) const
{
    return(engineNumber == rhs.engineNumber);
}

bool EngineID::operator!=(const EngineID& rhs) const
{
    return !((*this) == rhs);
}

int EngineID::getNumber() const
{
    return engineNumber;
}

void EngineID::setNumber(int number)
{
    engineNumber = number;
}