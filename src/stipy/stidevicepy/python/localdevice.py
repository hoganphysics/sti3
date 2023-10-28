from stipy.bin.stidevicepy import LocalDevice

from stipy.bin.stipybase import RawEvent
from stipy.bin.stidevicepy import SynchronousEventVector

import traceback

class EventParsingException(Exception):
    def __init__(self, event: RawEvent, message: str):
        self.event = event
        self.message = message
        super().__init__(self.message)

class EventConflictException(Exception):
    def __init__(self, event1: RawEvent, event2: RawEvent, message: str):
        self.event1 = event1
        self.event2 = event2
        self.message = message
        super().__init__(self.message)

def _parseEvents(self: LocalDevice, eventsIn: dict[float, list[RawEvent]], synchedEvents: SynchronousEventVector) -> None:
    '''User defined conversion of RawEvents to SynchronousEvent'''
    return

def _parseEventsWrapper(self, eventsIn, synchedEvents):
    try:
        self.parseEvents(eventsIn, synchedEvents)
    except EventParsingException as e:
        self.throwParsingException(e.event, e.message)
        raise ValueError()
    except EventConflictException as e:
        self.throwConflictException(e.event1, e.event2, e.message)
        raise ValueError()
    except Exception as e:
        self.throwPythonException(traceback.format_exc())
        raise e



setattr(LocalDevice, 'parseEventsWrapper', _parseEventsWrapper)
setattr(LocalDevice, 'parseEvents', _parseEvents)

