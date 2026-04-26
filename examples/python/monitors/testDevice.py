import math

import stipy.stidevicepy as stidevicepy


class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)

        self._sample_index = 0
        self._states = ["Idle", "Armed", "Running"]
        self._state_index = 0

        self.temperatureMonitor = self.addAutoMonitor(
            "Status/temperatureC",
            1.0,
            self.readTemperatureC
        ).addMetadata("units", "C") \
         .addMetadata("help", "AutoMonitor updated once per second.")

        self.stateMonitor = self.addMonitor("Status/state") \
            .addMetadata("help", "Regular LocalMonitor updated manually.") \
            .setValue(self._states[self._state_index])

        self.temperatureMonitor2 = self.addAutoMonitor(
            "Status/temperatureCos",
            1.0,
            self.readTemperatureCcos
        ).addMetadata("units", "C") \
         .addMetadata("help", "AutoMonitor updated once per second.")

    def readTemperatureC(self):
        self._sample_index += 1
        return round(22.0 + 1.5 * math.sin(self._sample_index / 4.0), 2)
    def readTemperatureCcos(self):
        self._sample_index += 1
        return round(22.0 + 1.5 * math.cos(self._sample_index / 4.0), 2)

    def advanceState(self):
        self._state_index = (self._state_index + 1) % len(self._states)
        self.stateMonitor.setValue(self._states[self._state_index])
