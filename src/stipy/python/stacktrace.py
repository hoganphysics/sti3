from inspect import getframeinfo, stack
from stipy.stipybase.stipybase import StackTrace as _StackTrace

def isFilename(name):
    if isinstance(name, str) :
        if len(name) > 0 and name[0] != "<":
            return True
    return False

def isFunctionName(name):
    if isinstance(name, str) :
        if len(name) > 0 and name[0] != "<":
            return True
    return False

# def foundLastFrame(functionName):
#     if functionName == "execute_file":
#         return True
#     if functionName == "makeshot":
#         return True
#     return False

def foundLastFrame(functionName):
    return functionName in ["execute_file", "makeshot", "_call_with_frames_removed", "load_module"]

def makeStackTrace():
    trace = _StackTrace()

    rawFrames = stack()
    startFrame = 2  # drop event(), etc

    if len(rawFrames) > 2:
        for i in range(startFrame, len(rawFrames)) :
            info = getframeinfo(rawFrames[i][0])
            if foundLastFrame(info.function):
                break
            # if not isFilename(info.filename) :
            #     break
            # if not isFunctionName(info.function):
            #     break
            trace.appendFrame(info.filename, info.lineno, info.function)

    return trace


