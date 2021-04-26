
#ifndef STI_PYTHON_PARSETICKET_H
#define STI_PYTHON_PARSETICKET_H


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

    ParseTicket();

    ResultTicket play();
    ResultTicket play(unsigned repeats);

    ParseTicket& wait();    //blocks until parse completes; returns this for chaining
    void cancel();   //cancels parse and stops wait()

private:


};


} //Python
} //STI

#endif

