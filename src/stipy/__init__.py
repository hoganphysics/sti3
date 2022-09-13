#import pkgutil
#__path__ = pkgutil.extend_path(__path__, __name__)

#import stidevicepy.stipybase as STIPy

#from .stipybase.stipybase import DeviceID, HubID, MixedValue, MixedValueType, ParseID, RawEvent, RawEventType, ShotConfig, ShotType, TimeStamp

#import stipy.stipybase as stipy
from stipy.stipybase import *
# from stipy.bin.stipy import *



# #import stipy.bin.stipy as _stipy
# from stipy.bin.stipy import dev as _dev
# from stipy.bin.stipy import *


# def dev(name, address, module) :
#     print("In dev 1")
#     #return _stipy.dev(name, address, module)
#     return _dev(name, address, module)



from stipy.bin.stipy import *
# from stipy.device import dev
# from stipy.event import event
# from stipy.var import setvar

from stipy.python.stacktrace import makeStackTrace
from stipy.python.group import RawEventGroup
from stipy.python.shot import STIPyShot

from stipy.python.stiglobal import *



# from stipy.stipybase import *

