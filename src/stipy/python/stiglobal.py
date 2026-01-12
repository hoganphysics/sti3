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
from stipy.stipybase.stipybase import HubID
from stipy.stipybase.stipybase import Configuration

from socket import gethostname as _gethostname
from getpass import getuser as _getuser
from socket import socket as _socket
from socket import AF_INET as _AF_INET
from socket import SOCK_DGRAM as _SOCK_DGRAM
from .ip_address import get_local_ip_address as _get_local_ip_address

# def get_local_ip_address():
#     s = _socket(_AF_INET, _SOCK_DGRAM)
#     try:
#         # Doesn't even have to be reachable
#         s.connect(('8.8.8.8', 1))
#         IP = s.getsockname()[0]
#     except Exception:
#         IP = '127.0.0.1'
#     finally:
#         s.close()
#     return IP

def get_local_ip_address():
    return _get_local_ip_address()

def connect(serverID, nameServerAddress=None, config=None, serverHubID=None):
    localAddress = _gethostname()
    username = _getuser()

    if nameServerAddress is None:
        configAddress = config.get("NetworkHub", "NameService") if config is not None else None
        if configAddress is not None:
            nameServerAddress = configAddress
        else:
            return None
    
    hubConfig = Configuration()
    hubConfig.set("omniORB", "endPoint", "giop:tcp::")
    hubConfig.set("omniORB", "endPointPublish", "giop:tcp:" + get_local_ip_address() + ":")
    hubConfig.set("omniORB", "clientConnectTimeOutPeriod", "500")

    if config is not None:
        hubConfig.append(config)

    if type(serverID) == str:
        serverID = DeviceID(serverID)

    if serverHubID is None:
        if hubConfig is not None:
            server = _connect(localAddress, serverID, nameServerAddress, hubConfig)
        else:
            server = _connect(localAddress, serverID, nameServerAddress)
    else:
        if type(serverHubID) == str:
            serverHubID = HubID(serverHubID)
        if hubConfig is not None:
            server = _connect(localAddress, serverID, serverHubID, nameServerAddress, hubConfig)
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


