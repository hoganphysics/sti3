from stipy.stidevicepy.stidevicepy import Device
from stipy.stidevicepy.stidevicepy import LogBrowser
from stipy.stidevicepy.stidevicepy import LogManager


_get_log_manager = Device.getLogManager


def getLogManager(self: Device):
    manager = _get_log_manager(self)
    if manager is not None:
        manager._owner_device = self
    return manager


def openLog(self: LogManager, logID, tail_lines=200):
    owner_device = getattr(self, "_owner_device", None)
    if owner_device is None:
        raise ValueError(
            "LogManager.openLog() requires a manager obtained from Device.getLogManager() "
            "so the originating device can be resolved."
        )
    return owner_device.openLog(logID, tail_lines=tail_lines)


def page_backward(self: LogBrowser, lines=0):
    return self.pageBackward(lines)


def page_forward(self: LogBrowser, lines=0):
    return self.pageForward(lines)


def save_local(self: LogBrowser, path):
    return self.saveLocal(path)


def refresh_metadata(self: LogBrowser):
    return self.refreshMetadata()


setattr(Device, "getLogManager", getLogManager)
setattr(LogManager, "openLog", openLog)

setattr(LogBrowser, "page_backward", page_backward)
setattr(LogBrowser, "page_forward", page_forward)
setattr(LogBrowser, "save_local", save_local)
setattr(LogBrowser, "refresh_metadata", refresh_metadata)
