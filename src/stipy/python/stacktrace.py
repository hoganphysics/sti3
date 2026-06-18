import sys

from stipy.stipybase.stipybase import StackTrace as _StackTrace

_LAST_FRAME_FUNCTIONS = {"execute_file", "makeshot", "_call_with_frames_removed", "load_module"}

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
    return functionName in _LAST_FRAME_FUNCTIONS

def makeStackTrace():
    trace = _StackTrace()

    try:
        frame = sys._getframe(2)  # drop makeStackTrace() and event(), setvar(), etc.
    except ValueError:
        return trace

    while frame is not None:
        code = frame.f_code
        function = code.co_name
        if foundLastFrame(function):
            break
        trace.appendFrame(code.co_filename, frame.f_lineno, function)
        frame = frame.f_back

    return trace

