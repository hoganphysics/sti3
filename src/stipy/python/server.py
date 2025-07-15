from stipy.bin.stipy import STIPyServer
from stipy.bin.stipybase import Sequence
from stipy.bin.stipybase import SequenceType
from stipy.bin.stipy import ParsedVar
from stipy.bin.stipybase import RawEventGroup
from stipy.bin.stipybase import SequenceEntryID
from stipy.bin.stipybase import SequenceID

_makeshot = STIPyServer.makeshot

# _makesequence = STIPyServer.makesequence

# def makesequence(varsTable=None):
#     seq = _makesequence()
#     seq.repeats = 0

#     if varsTable == None:
#         return seq
    
#     if (not (type(varsTable) is list)):
#         raise ValueError("Sequence table must be a list.")
    
#     seq.type = SequenceType.Closed

#     for entry in varsTable:
#         if (type(entry) is dict):
#             g = RawEventGroup()
#             for key in entry.keys():
#                 g.setvar(key, entry[key])
#                 # v = ParsedVar(key, entry[key])
#                 # vars.add(v)
#             seq.append(set(g.getVars()))
#         elif (type(entry) is set):
#             seq.append(entry)
#         else:
#             raise ValueError("Sequence table entries must be a set or a dictionary.")
#     return seq

# class SequenceTicket:
#     def __init__(self):
#         return

# setattr(STIPyServer, 'makesequence', makesequence)

def _makeshot_file(self, filename, vars=None):

    def execute_file():
        with open(filename) as file:
            code = compile(file.read(), filename, 'exec')
            exec(code, globals(), locals())
    
    if vars == None:
        return _makeshot(self, execute_file)
    else:
        return _makeshot(self, execute_file, vars)


def makeshot(self, source=None, vars=None):
    if source == None and vars == None:
        return self.makeshot()
    elif type(source) == str:
        return _makeshot_file(self, source, vars)
    elif callable(source):
        if vars == None:
            return _makeshot(self, source)
        else:
            return _makeshot(self, source, vars)
    else:
        raise ValueError("Source must be a string filename or a callable.")



def run(self, sequence: Sequence, sequenceID: SequenceID):
    print(sequence.sequenceTable)

    shots = []

    for key in sequence.sequenceTable.keys():
        shot = makeshot(self, sequence.shotmaker, sequence.sequenceTable[key].overwritten)
        shots.append(shot)

        seqEntryID = SequenceEntryID()
        seqEntryID.seqID = sequenceID
        seqEntryID.seqIndex = key

        parseTick = self.parse(shot, seqEntryID)
        parseTick.wait()

        resultTick = self.play(parseTick)
        resultTick.wait()

    return

setattr(STIPyServer, 'run', run)
setattr(STIPyServer, 'makeshot', makeshot)