from stipy import *
from testdir.sub import *
from inspect import getframeinfo, stack

def f():
    server=dev("STI Server", "localhost", 0)
    # event(ch(server, 1), 500, 67.9)
    
    d1=dev("TestDevice", "localhost2", 0)
    c2=ch(d1, 2)

    meas(c2, 200)

    g(c2)

    c1=ch(d1, 1)

    # event(c1, 500, 67.9)

def f2():
    rawFrames = stack()
    return rawFrames

def f3():
    rawFrames = g2()
    return rawFrames