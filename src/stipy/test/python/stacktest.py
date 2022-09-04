
from inspect import currentframe, getframeinfo, stack

def createMOT(time) :
    cf = currentframe()
    # print(getframeinfo(cf))
    # print("Creating MOT: "+str(time))
#    print(__file__)
#    finfo = getframeinfo(cf)
#    print(finfo.filename, finfo.lineno)
#    print(cf.f_back.f_lineno)
#    print(finfo)
#    print(getframeinfo(cf).filename, getframeinfo(cf).lineno)
#    print(getframeinfo(cf.f_back).filename, getframeinfo(cf.f_back).lineno)
    return cf
    # print(getframeinfo(cf.f_back))
    # print("Done")
    # print(stack())

def g():
    makeStackTrace()

def f():
    g()

def isFilename(name):
    if isinstance(name, str) :
        if len(name) > 0 and name[0] != "<":
            return True
    return False

def makeStackTrace():
    rawFrames = stack()
    for frame in rawFrames :
        info = getframeinfo(frame[0])
        print(str(isFilename(info.filename)))
        print([info.filename, info.lineno, info.function])
        if not isFilename(info.filename) :
            break
