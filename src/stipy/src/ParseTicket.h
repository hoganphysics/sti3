
#ifndef STI_PYTHON_PARSETICKET_H
#define STI_PYTHON_PARSETICKET_H

#include "ParseID.h"

#include <string>

namespace STI
{
namespace Python
{


class ResultTicket;
class ParseTicket;


class ParseTicket
{
public:

    ParseTicket(const STI::Engine::ParseID& id);

    ResultTicket play();
    ResultTicket play(unsigned repeats);

    ParseTicket& wait();    //blocks until parse completes; returns this for chaining
    void cancel();   //cancels parse and stops wait()

    const STI::Engine::ParseID& getParseID() const;

private:

    STI::Engine::ParseID pid;

};


} //Python
} //STI

#endif

