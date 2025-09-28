from stipy.stipy import STIPyShot
from stipy.python.stacktrace import makeStackTrace as _makeStackTrace
from stipy.stipybase.stipybase import MixedValue
from stipy.stipybase.stipybase import MixedValueType

_setvar = STIPyShot.setvar
_event = STIPyShot.event
_meas = STIPyShot.meas


def var(self, fullVarName) :  
    return self.rootgroup().var(fullVarName)

def setvar(self, fullVarName, value, group=""):
    return _setvar(self, fullVarName, value, _makeStackTrace(), group)
def settag(self, fullVarName, group="") :
    return self.group(group).addtag(fullVarName)
def event(self, target, time, value, group="") :
    return _event(self, target, time, value, _makeStackTrace(), group)
def meas(self, target, time, value=MixedValue(), group="") :
    if type(value) == MixedValue and value.isType(MixedValueType.Empty):
        return _meas(self, target, time, _makeStackTrace(), group)
    else:
        return _meas(self, target, time, value, _makeStackTrace(), group)


setattr(STIPyShot, 'var', var)

setattr(STIPyShot, 'setvar', setvar)
setattr(STIPyShot, 'settag', settag)
setattr(STIPyShot, 'event', event)
setattr(STIPyShot, 'meas', meas)
