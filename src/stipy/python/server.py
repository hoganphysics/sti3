from stipy.bin.stipy import STIPyServer
from stipy.bin.stipybase import Sequence
from stipy.bin.stipybase import SequenceType
from stipy.bin.stipy import ParsedVar
from stipy.bin.stipybase import RawEventGroup

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

def run(self, sequence: Sequence):
    print(sequence.sequenceTable)

    shots = []

    for key in sequence.sequenceTable.keys():
        shot = self.makeshot(sequence.shotmaker, sequence.sequenceTable[key].overwritten)
        shots.append(shot)

        parseTick = self.parse(shot)
        parseTick.wait()

        resultTick = self.play(parseTick)
        resultTick.wait()

    return

setattr(STIPyServer, 'run', run)
