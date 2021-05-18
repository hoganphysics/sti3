
#import sys

import inspect

import getpass
import socket

def getUser() :
    hostname = socket.gethostname()
    print(hostname)
    username = getpass.getuser()
    print(username)
    return

def printfunc() :
    stk = inspect.stack()[1]
    mod = inspect.getmodule(stk[0])
    print("Currently in {}.{}".format(mod, stk[3]))
    print(inspect.stack()[2])

class Var :
    def __init__(self, name, value):
        self.name = name
        self.value = value

class Channel :
    def __init__(self, device, number):
        self.device = device
        self.number = number
    
class Device :
    def __init__(self, name, address, module, targetserver):
        self.name = name
        self.address = address
        self.module = module
        self.targetserver = targetserver

class STIEvent :
    def __init__(self, time, channel, value):
        self.time = time
        self.channel = channel
        self.value = value

class STIServer :
    def __init__(self):
        printfunc()
        self.clear()
    
    def clear(self):
        self.channels = []
        self.devices = []
        self.vars = []
        self.events = []

    def parse(self, func):
        self.clear()
        global __parsedChannels__
        global __parsedDevices__
        global __parsedVars__
        global __parsingEvents__
        __parsedChannels__ = self.channels
        __parsedDevices__ = self.devices
        __parsedVars__ = self.vars
        __parsingEvents__ = self.events
        func()

    

def setvar(name, value) :
    newvar = Var(name, value)
    global __parsedVars__
    #globals()[name] = value

    if(newvar.name in [x.name for x in __parsedVars__]) :
        print("Error: setvar '" + name + "' has already been defined.")
    else :
        #thismodule = sys.modules[__name__]
        #setattr(thismodule, name, value)
        globals()[name] = value
        __parsedVars__.append(newvar)

def ch(device, number) :
    channel = Channel(device, number)
    global __parsedChannels__
    __parsedChannels__.append(channel)
    return channel

def dev(name, address, module, targetserver) :
    device = Device(name, address, module, targetserver)
    global __parsedDevices__
    __parsedDevices__.append(device)
    return device

def parse(function) :
    global __parsedEvents__
    global __parsedChannels__
    __parsedEvents__.clear()
    __parsedChannels__.clear()

def event(time, channel, value) :
    # print(inspect.stack())
    global __parsingEvents__
    __parsingEvents__.append(STIEvent(time, channel, value))

