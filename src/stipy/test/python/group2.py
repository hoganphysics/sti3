
from stipy.bin.stipy import RawEventGroup as _RawEventGroup
from stipy.python.stacktrace import makeStackTrace
# from stipy.bin.stipy import pybind11_object
# from stipy.bin.stipy import RawEventGroup as _addvar



class RawEventGroup(_RawEventGroup):
    def __getattribute__(self, attr):
        return _RawEventGroup.__getattribute__(self, attr)

    def __setattr__(self, attr, value):
        return _RawEventGroup.__setattr__(self, attr, value)

    def var(self, fullVarName) :
        return _RawEventGroup.var(self, fullVarName, makeStackTrace())
    def addvar(self, fullVarName, value) :
        # print("In python addvar2()")
        return _RawEventGroup.addvar(self, fullVarName, value, makeStackTrace())
    def addtag(self, fullVarName) :
        return _RawEventGroup.addtag(self, fullVarName, makeStackTrace())
    def addEvent(self, target, time, value) :
        return _RawEventGroup.addEvent(self, target, time, value, makeStackTrace())
    def addMeas(self, target, time, value) :
        return _RawEventGroup.addMeas(self, target, time, value, makeStackTrace())
    # def group(self, groupName) :
    #     g = _RawEventGroup.group(self, groupName)
    #     g.__class__ = RawEventGroup
    #     return g


# https://stackoverflow.com/questions/3464061/cast-base-class-to-derived-class-python-or-more-pythonic-way-of-extending-class

origialnew = _RawEventGroup.__new__

def __new__(cls, name, parentName):
    print(cls)
    print(_RawEventGroup)
    if cls == _RawEventGroup:
        print("class match")
        return object.__new__(RawEventGroup, name, parentName)
    return origialnew(cls, name, parentName)

# We substitute the __new__ method of the nx.Graph class
# with our own.     
_RawEventGroup.__new__ = staticmethod(__new__)


# setattr(RawEventGroup, 'addvar', addvar)

def new_bookcol_method(self):
    pass

base_classes.Bookcollection.__dict__["old_bookcol_method"] = new_bookcol_method