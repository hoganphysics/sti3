import stipy
import stipy.stidevicepy as stidevicepy


class AnalysisDevice(stidevicepy.LocalDevice):
    """A device that hosts post-processing targets.

    A post-processing *target* is a named analysis routine that runs after a shot
    finishes playing, on a background worker thread, without blocking the parsing
    or playback of later shots.  A common pattern is a small, dedicated analysis
    device whose only job is to host targets.

    A timing file requests post-processing against a target with ``postTarget()``
    and ``postProcess()``.  Note the deliberate absence of a time argument: a
    ``postProcess()`` request is not hard-timed.

        from stipy import *

        analysis = postTarget("AnalysisDevice", "atom number")

        def shotmaker():
            meas(ch(camera, 0), 10_000_000)
            postProcess(analysis, {"roi": [10, 20, 100, 100], "model": "gaussian"})
    """

    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)

        # A measurement (input) channel.  In this single-device example the same
        # device plays the shot and post-processes it, so its own
        # PersistenceManager holds the shot data the targets analyze.
        self.addInputChannel(0, stipy.MixedValueType.Number, "signal")

        # Register post-processing targets.  The callback receives the completed
        # shot's ShotID and the per-request options dict, and returns a results
        # dict that is broadcast in a PostProcessingComplete device message.  If
        # the callback raises, the failure is reported in that message instead of
        # crashing the worker.
        self.addPostProcessingTarget(
            "atom number",
            self.fitAtomNumber,
            "Reduces a shot's measurement data to an atom number.",
        )

        # A target can also be a simple lambda.
        self.addPostProcessingTarget(
            "echo options",
            lambda shotID, options: dict(options),
            "Returns the request options unchanged (useful for testing).",
        )

    def fitAtomNumber(self, shotID, options):
        # Pull this shot's data by ShotID.  The owning device's PersistenceManager
        # holds the result; dispatch happens after the result is persisted, so the
        # lookup succeeds.  A real target would fit or reduce the data here.
        shotResult = self.getPersistenceManager().getShotResult(shotID)
        haveData = shotResult is not None

        return {
            "model": options.get("model", "none"),
            "haveShotData": haveData,
            "N": 1.0e6,
        }
