
#ifndef STI_PYTHON_PYPARSETICKET_H
#define STI_PYTHON_PYPARSETICKET_H


#include "ParseTicket.h"


namespace STI
{
namespace Python
{


class PyParseTicket : public STI::Engine::ParseTicket
{
public:

    PyParseTicket(const STI::Engine::ParseID& id, 
                const std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
 
private:

    bool waitCheck();

};


} //Python
} //STI

#endif


