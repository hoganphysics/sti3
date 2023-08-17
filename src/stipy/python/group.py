
from stipy.bin.stipybase import RawEventGroup
from stipy.python.stacktrace import makeStackTrace

_group = RawEventGroup.group
_var = RawEventGroup.var
_addvar = RawEventGroup.addvar
_addtag = RawEventGroup.addtag
_addEvent = RawEventGroup.addEvent
_addMeas = RawEventGroup.addMeas


def group(self, name, color="") :
    g = _group(self, name)
    if color != "" :
        g.setcolor(color)
    return g

def setcolor(self, color) :
    self.addMetaData("color", color)

def var(self, fullVarName) :
    v = _var(self, fullVarName, makeStackTrace())
    if v.isBound():
        return v.value().getValue()
    else:
        return v

def setvar(self, fullVarName, value):
    return _addvar(self, fullVarName, value, makeStackTrace())
def settag(self, fullVarName) :
    return _addtag(self, fullVarName, makeStackTrace())
def event(self, target, time, value) :
    return _addEvent(self, target, time, value, makeStackTrace())
def meas(self, target, time, value) :
    return _addMeas(self, target, time, value, makeStackTrace())



setattr(RawEventGroup, 'group', group)
setattr(RawEventGroup, 'setcolor', setcolor)

setattr(RawEventGroup, 'var', var)
setattr(RawEventGroup, 'setvar', setvar)
setattr(RawEventGroup, 'settag', settag)
setattr(RawEventGroup, 'event', event)
setattr(RawEventGroup, 'meas', meas)

