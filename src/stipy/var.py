

# from stipy.bin.stipy import STIPyShot.setvar as STIPyShot._setvar

from stipy.bin.stipy import STIPyShot

from stipy.stipybase import StackTrace as _StackTrace
# from stipy.stipybase import StackTrace as _StackTrace


def setvar(self, name, value) :
    print("In python setvar2()")
    trace = _StackTrace()
    trace.appendFrame("tmp.py", 23, "makeMOT")
    return self.setvar(name, value, trace)

setattr(STIPyShot, 'setvar2', setvar)

