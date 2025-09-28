from stipy.stipybase.stipybase import RawEvent
from stipy.stipybase.stipybase import RawEventType


def print(self):
    if self.type() == RawEventType.Measurement:
        header = "meas"
    elif self.type() == RawEventType.Waveform:
        header = "waveform"
    else:
        header = "event"

    return header + "(Time=" + self.printTime() \
            + ", Channel=" + str(self.target()) \
            + ", Value=" + str(self.value()) + ")"


setattr(RawEvent, '__repr__', print)

