from stipy.bin.stidevicepy import Logger

from typing import Callable


def addReadLogTask(self: Logger, channel: int, timeInterval: str, value=None):
    if value == None:
        self.__addReadLogTask(channel, timeInterval)
    elif isinstance(value, Callable):
        self.__addReadLogTask_callable(channel, timeInterval, value)
    else:
        self.__addReadLogTask_value(channel, timeInterval, value)

def addWriteLogTask(self: Logger, channel: int, timeInterval: str, value):
    if isinstance(value, Callable):
        self.__addWriteLogTask_callable(channel, timeInterval, value)
    else:
        self.__addWriteLogTask_value(channel, timeInterval, value)


setattr(Logger, 'addReadLogTask', addReadLogTask)
setattr(Logger, 'addWriteLogTask', addWriteLogTask)
