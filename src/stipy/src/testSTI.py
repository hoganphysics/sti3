
import inspect

def testSum(x,y):
    return x+2*y

def event(time, channel, value) :
    # print(inspect.stack())
    global __parsingEvents__
    __parsingEvents__.append(STIEvent(time, channel, value))

def parse(function) :
    global __parsingEvents__
    STI.__parsingEvents__.clear()

class STIServer:
    def __init__(self):
        self.events = []

    def parse(self):
        global __parsingEvents__
        __parsingEvents__ = self.events
    def parse2(self, func):
        global __parsingEvents__
        __parsingEvents__ = self.events
        func()


class STIEvent :
    def __init__(self, time, channel, value):
        self.time = time
        self.channel = channel
        self.value = value

    def toString(self) :
        print(self.time)
        print(self.channel)
        print(self.value)

class AbstractEvent():
    def __init__(self, time, channel, value):
        self.time = time
        self.channel = channel
        self.value = value
