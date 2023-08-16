from stipy.bin.stipy import setvar as _setvar
from stipy.bin.stipy import var as _var
from stipy.bin.stipy import settag as _settag
from stipy.bin.stipy import event as _event
from stipy.bin.stipy import meas as _meas
from stipy.bin.stipy import group as _group
from stipy.bin.stipy import connect as _connect
#from stipy.bin.stipy import ch as _ch
from stipy.bin.stipy import STIPyServer
# from stipy.bin.stipybase import Sequence
from stipy.stipybase.python.sequence import STIPySequence

from stipy.python.stacktrace import makeStackTrace

from socket import gethostname
from getpass import getuser


def connect(serverID, nameServerAddress, serverHubID=None):
    localAddress = gethostname()
    username = getuser()

    if serverHubID == None:
        server = _connect(localAddress, serverID, nameServerAddress)
    else:
        server = _connect(localAddress, serverID, serverHubID, nameServerAddress)

    if server != None and type(server) == STIPyServer:
        server.set_username(username)

    return server


def group(name, color="") :
    g = _group(name)
    if color != "" :
        g.setcolor(color)
    return g

def var(name) :
    v = _var(name, makeStackTrace())
    if v.isBound():
        return v.value().getValue()
    else:
        return v



def setvar(name, value, group="") :
    return _setvar(name, value, makeStackTrace(), group)
def settag(name, group="") :
    return _settag(name, makeStackTrace(), group)

def event(channel, time, value, group="") :
    return _event(channel, time, value, makeStackTrace(), group)
def meas(channel, time, value, group="") :
    return _meas(channel, time, value, makeStackTrace(), group)
def meas(channel, time, group="") :
    return _meas(channel, time, makeStackTrace(), group)

def makesequence(shotmaker, varsTable=None) :
    return STIPySequence(shotmaker, varsTable)


