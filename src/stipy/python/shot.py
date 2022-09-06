from stipy.bin.stipy import STIPyShot
from stipy.python.stacktrace import makeStackTrace
from stipy.bin.stipybase import MixedValue
from stipy.bin.stipybase import MixedValueType

_setvar = STIPyShot.setvar
_event = STIPyShot.event
_meas = STIPyShot.meas


def var(self, fullVarName) :  
    return self.group().var(fullVarName)

def setvar(self, fullVarName, value, group=""):
    return _setvar(self, fullVarName, value, makeStackTrace(), group)
def settag(self, fullVarName, group="") :
    return self.group(group).addtag(fullVarName)
def event(self, target, time, value, group="") :
    return _event(self, target, time, value, makeStackTrace(), group)
def meas(self, target, time, value=MixedValue(), group="") :
    if type(value) == MixedValue and value.isType(MixedValueType.Empty):
        return _meas(self, target, time, makeStackTrace(), group)
    else:
        return _meas(self, target, time, value, makeStackTrace(), group)


setattr(STIPyShot, 'var', var)

setattr(STIPyShot, 'setvar', setvar)
setattr(STIPyShot, 'settag', settag)
setattr(STIPyShot, 'event', event)
setattr(STIPyShot, 'meas', meas)
