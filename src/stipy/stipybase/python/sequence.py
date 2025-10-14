from ..stipybase import Sequence
from ..stipybase import SequenceType
from ..stipybase import RawEventGroup
from ..stipybase import ShotType
# from stipy.bin.stipy import ParsedVar
# _sequence__init__ = Sequence.__init__

from socket import gethostname as _gethostname
from getpass import getuser as _getuser

def init(self, shotmaker, varsTable=None):
    self.repeats = 0
    self.shotmaker = shotmaker

    if varsTable == None:
        return _sequence__init__(self)
    
    if (not (type(varsTable) is list)):
        raise ValueError("Sequence table must be a list.")
    
    # seq.type = SequenceType.Closed
    _sequence__init__(self, SequenceType.Closed)

    for entry in varsTable:
        if (type(entry) is dict):
            vars={}
            for key in entry.keys():
                v = ParsedVar(key, entry[key])
                vars.add(v)
            self.append(vars)
        elif (type(entry) is set):
            self.append(entry)
        else:
            raise ValueError("Sequence table entries must be a set or a dictionary.")


# setattr(Sequence, '__init__', init)


def getAllVars(eventGroup: RawEventGroup):
    if (type(eventGroup) != RawEventGroup):
        return set()
    
    vars = set(eventGroup.vars())

    subgroups = eventGroup.subgroups()
    if len(subgroups) > 0:
        for g in eventGroup.subgroups():
            vars.update(getAllVars(g))
    
    return vars

def getAllOverwrittenVars(eventGroup: RawEventGroup):
    if (type(eventGroup) != RawEventGroup):
        return set()
    
    ovars = eventGroup.overwrittenVars()

    subgroups = eventGroup.subgroups()
    if len(subgroups) > 0:
        for g in subgroups:
            ovars.update(getAllOverwrittenVars(g))
    
    return ovars


class STIPySequence(Sequence):
    def __init__(self, shotmaker, varsTable = None, description: str = ""):
        self.repeats = 0
        self.shotmaker = shotmaker
        self.basegroup = RawEventGroup()

        # self._table_lock = threading.RLock()

        localAddress = _gethostname()
        username = _getuser()
        
        if varsTable == None:
            Sequence.__init__(self, SequenceType.Open)
        else:
            Sequence.__init__(self, SequenceType.Closed)

        if hasattr(self, 'shotConfig'):
            self.shotConfig.file = str(self.shotmaker)
            self.shotConfig.comment = description
            self.shotConfig.shotType = ShotType.Sequence
            self.shotConfig.jobSourceID.machine = localAddress
            self.shotConfig.jobSourceID.user = username
        
        if varsTable == None:
            return

        if (not (type(varsTable) is list)):
            raise ValueError("Sequence table must be a list.")

        for entry in varsTable:
            if (type(entry) is dict):
                vars={}
                for key in entry.keys():
                    self.basegroup.bindvar(key, entry[key])
                    # v = ParsedVar(key, entry[key])
                    # vars.add(v)
                self.append(self.overwrittenVars())
            elif (type(entry) is set):
                self.append(entry)
            else:
                raise ValueError("Sequence table entries must be a set or a dictionary.")

    def vars(self):
        return getAllVars(self.basegroup)

    def overwrittenVars(self):
        return getAllOverwrittenVars(self.basegroup)

    # def __getvars(eventGroup: RawEventGroup):
    #     if (type(eventGroup) != RawEventGroup):
    #         return set()
        
    #     vars = set(eventGroup.getVars())

    #     if len(eventGroup.getSubgroups()) > 0:
    #         for g in eventGroup.getSubgroups():
    #             vars.update(STIPySequence.__getvars(g))
        
    #     return vars
        





