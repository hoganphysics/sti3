from inspect import getframeinfo, stack
from stipy.stipybase import StackTrace as _StackTrace

def isFilename(name):
    if isinstance(name, str) :
        if len(name) > 0 and name[0] != "<":
            return True
    return False

def makeStackTrace():
    trace = _StackTrace()

    rawFrames = stack()
    for frame in rawFrames :
        info = getframeinfo(frame[0])
        trace.appendFrame(info.filename, info.lineno, info.function)
        if not isFilename(info.filename) :
            break

    return trace


