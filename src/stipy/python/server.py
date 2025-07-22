from stipy.bin.stipy import STIPyServer
from stipy.bin.stipybase import Sequence
from stipy.bin.stipybase import SequenceType
from stipy.bin.stipy import ParsedVar
from stipy.bin.stipybase import RawEventGroup
from stipy.bin.stipybase import SequenceEntryID
from stipy.bin.stipybase import SequenceID

from collections.abc import Callable
import importlib.util, sys, pathlib

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



def load_module2(path, name=None):
    spec  = importlib.util.spec_from_file_location(name or pathlib.Path(path).stem, path)
    mod   = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = mod
    spec.loader.exec_module(mod)
    return mod

def load_module(path, name=None):
    path = pathlib.Path(path).resolve()

    # 1️⃣  Make the file’s directory importable
    dir_path = str(path.parent)
    if dir_path not in sys.path:          # avoid duplicates
        sys.path.insert(0, dir_path)      # search first, like real scripts

    # 2️⃣  Load the file as a proper module
    spec   = importlib.util.spec_from_file_location(name or path.stem, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module       # so future plain ‘import name’ works
    spec.loader.exec_module(module)
    return module

def _makeshot_file(self, filename, vars=None):

    # def execute_file():
    #     with open(filename) as file:
    #         code = compile(file.read(), filename, 'exec')
    #         exec(code, globals(), locals())
    
    # def execute_file():
    #     namespace = {
    #         '__name__': '__main__',
    #         '__file__': filename,
    #         '__package__': None,
    #         '__cached__': None,
    #     }
    #     with open(filename, 'r', encoding='utf-8') as file:
    #         code = compile(file.read(), filename, 'exec')
    #         exec(code, namespace, namespace)
    
    def execute_file():
        load_module(filename)

    if vars == None:
        return _makeshot(self, execute_file)
    else:
        return _makeshotVars(self, execute_file, vars)

def _makeshotVars(self, shotmaker, vars):
    g = RawEventGroup()

    if (type(vars) is dict):
        [g.bindvar(k, v) for k,v in vars.items()]
    elif (type(vars) is set):
        [g.bindvar(v.name, v.value()) for v in vars]
    
    return _makeshot(self, shotmaker, g.overwrittenVars())

def makeshot(self, source=None, vars=None):


    if source == None and vars == None:
        return self.makeshot()
    elif type(source) == str:
        return _makeshot_file(self, source, vars)
    elif callable(source):
        if vars == None:
            return _makeshot(self, source)
        else:
            return _makeshotVars(self, source, vars)
    else:
        raise ValueError("Source must be a string filename or a callable.")



def run(self, sequence: Sequence, progress: Callable[[int, int], None] = None):

    sequenceID = self.addSequence(sequence)
    # print(sequence.sequenceTable)
    print(sequenceID)

    shots = []
    shot_number = 0
    

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

        if progress is not None:
            shot_number += 1
            progress(shot_number, len(sequence.sequenceTable))

    return

setattr(STIPyServer, 'run', run)
setattr(STIPyServer, 'makeshot', makeshot)