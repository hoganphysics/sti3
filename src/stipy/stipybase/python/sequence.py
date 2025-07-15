from stipy.bin.stipybase import Sequence
from stipy.bin.stipybase import SequenceType
from stipy.bin.stipybase import RawEventGroup


# _sequence__init__ = Sequence.__init__

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
    def __init__(self, shotmaker, varsTable = None):
        self.repeats = 0
        self.shotmaker = shotmaker
        self.basegroup = RawEventGroup()

        if varsTable == None:
            return Sequence.__init__(self)
        
        if (not (type(varsTable) is list)):
            raise ValueError("Sequence table must be a list.")
        
        # seq.type = SequenceType.Closed
        Sequence.__init__(self, SequenceType.Closed)

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
        





