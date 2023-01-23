from stipy import *

def f():
    server=dev("STI Server", "localhost", 0)
    # event(ch(server, 1), 500, 67.9)
    
    d1=dev("TestDevice", "localhost2", 0)
    c2=ch(d1, 2)

    meas(c2, 200)

    c1=ch(d1, 1)

    # event(c1, 500, 67.9)
