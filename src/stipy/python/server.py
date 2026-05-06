from collections.abc import Callable

from stipy.stipy import STIPyServer
from stipy.stipybase.stipybase import SequenceEntryID
from stipy.stipybase.stipybase import SequenceID
from stipy.stipybase.stipybase import ShotType
from stipy.stipybase.python.sequence import STIPySequence
from stipy.python.makeshot import make_shot


_makeshot = STIPyServer.makeshot


def makeshot(self, source=None, vars=None, shot_type=None):
    return make_shot(_makeshot, self, source, vars, shot_type)


def run_shots(self, sequence: STIPySequence, sequenceID: SequenceID, progress: Callable[[int, int], None] = None):
    getKeyAttempts = 3
    while getKeyAttempts > 0:
        try:
            keys = list(sequence.sequenceTable.keys())
        except RuntimeError:
            getKeyAttempts -= 1
            if getKeyAttempts == 0:
                raise
        else:
            break

    shot_number = 0

    for key in keys:
        entry = sequence.sequenceTable.get(key)

        if entry is None:
            continue

        shot = makeshot(self, sequence.shotmaker, entry.overwritten, shot_type=ShotType.SequenceEntry)

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


def run(self, sequence: STIPySequence, progress: Callable[[int, int], None] = None):
    sequenceID = self.addSequence(sequence)
    run_shots(self, sequence, sequenceID, progress)


setattr(STIPyServer, "run", run)
setattr(STIPyServer, "run_shots", run_shots)
setattr(STIPyServer, "makeshot", makeshot)
