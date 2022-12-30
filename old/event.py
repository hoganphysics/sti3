

from stipy.bin.stipy import event as _event

from stipy.stipybase import StackTrace as _StackTrace

def event(channel, time, value, group) :
    print("In python event()")
    trace = _StackTrace(1, 2, 3)
    #return _stipy.dev(name, address, module)
    return _event(channel, time, value, trace, group)


