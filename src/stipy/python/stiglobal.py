from stipy.stipy import setvar as _setvar
from stipy.stipy import var as _var
from stipy.stipy import settag as _settag
from stipy.stipy import event as _event
from stipy.stipy import meas as _meas
from stipy.stipy import set_trigger as _set_trigger
from stipy.stipy import group as _group
from stipy.stipy import connect as _connect
from stipy.stipy import STIPyServer
# from stipy.bin.stipybase import Sequence
from stipy.stipybase.python.sequence import STIPySequence
from stipy.python.stacktrace import makeStackTrace as _makeStackTrace
from stipy.stipybase.stipybase import DeviceID

from socket import gethostname as _gethostname
from getpass import getuser as _getuser


def connect(serverID, nameServerAddress, serverHubID=None):
    localAddress = _gethostname()
    username = _getuser()

    if type(serverID) == str:
        serverID = DeviceID(serverID)

    if serverHubID == None:
        server = _connect(localAddress, serverID, nameServerAddress)
    else:
        server = _connect(localAddress, serverID, serverHubID, nameServerAddress)

    if server != None and type(server) == STIPyServer:
        server.setHostname(localAddress)
        server.setUsername(username)

    return server


def group(name, color="") :
    g = _group(name)
    if color != "" :
        g.setcolor(color)
    return g

def var(name) :
    v = _var(name, _makeStackTrace())
    if v.isBound():
        return v.value().getValue()
    else:
        return v



def setvar(name, value, group="") :
    return _setvar(name, value, _makeStackTrace(), group)
def settag(name, group="") :
    return _settag(name, _makeStackTrace(), group)

def event(channel, time, value, group="") :
    return _event(channel, time, value, _makeStackTrace(), group)
def meas(channel, time, value=None, group="") :
    if value == None:
        return _meas(channel, time, _makeStackTrace(), group)
    else:
        return _meas(channel, time, value, _makeStackTrace(), group)
#def meas(channel, time, group="") :
#    return _meas(channel, time, _makeStackTrace(), group)

def set_trigger(device):
    return _set_trigger(device, _makeStackTrace())

def makesequence(shotmaker, varsTable=None, description: str = ""):
    return STIPySequence(shotmaker, varsTable, description=description)


