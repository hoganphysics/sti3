from stipy.bin.stipy import setvar as _setvar
from stipy.bin.stipy import var as _var
from stipy.bin.stipy import settag as _settag
from stipy.bin.stipy import event as _event
from stipy.bin.stipy import meas as _meas
from stipy.bin.stipy import group as _group

from stipy.python.stacktrace import makeStackTrace


def group(name, color="") :
    g = _group(name)
    if color != "" :
        g.setcolor(color)
    return g

def var(name) :
    v = _var(name, makeStackTrace())
    if v.isBound():
        return v.value().getValue()
    else:
        return v

def setvar(name, value, group="") :
    return _setvar(name, value, makeStackTrace(), group)
def settag(name, group="") :
    return _settag(name, makeStackTrace(), group)

def event(channel, time, value, group="") :
    return _event(channel, time, value, makeStackTrace(), group)
def meas(channel, time, value, group="") :
    return _meas(channel, time, value, makeStackTrace(), group)
def meas(channel, time, group="") :
    return _meas(channel, time, makeStackTrace(), group)
