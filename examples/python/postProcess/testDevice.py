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
        # device plays the shot and post-processes it, so it is its own shot owner
        # and no addPartner() is needed.  (A dedicated analysis device that analyzes
        # shots played by *other* devices declares each owner with addPartner() so
        # the worker can pull their ShotResult.)
        self.addInputChannel(0, stipy.MixedValueType.Number, "signal")

        # Register post-processing targets.  The callback receives the completed
        # shot's pulled ShotResult (the worker resolved it from the owning device)
        # and the per-request options dict, and returns a results dict that is
        # broadcast in a PostProcessingComplete device message.  If the callback
        # raises, the failure is reported in that message instead of crashing the
        # worker.
        #
        # addPostProcessingTarget() returns a builder whose addOption() calls chain
        # to declare the options the target understands.  These hints are
        # documentation only (they are not validated) -- they let a caller discover,
        # via getPostProcessingTargets(), what to pass in a postProcess() payload.
        self.addPostProcessingTarget(
            "atom number",
            self.fitAtomNumber,
            "Reduces a shot's measurement data to an atom number.",
        ) \
            .addOption("roi", "Region of interest [x, y, w, h] to integrate.") \
            .addOption("model", "Fit model name, e.g. 'gaussian'.")

        # A target can also be a simple lambda.
        self.addPostProcessingTarget(
            "echo options",
            lambda shotResult, options: dict(options),
            "Returns the request options unchanged (useful for testing).",
        )

    def fitAtomNumber(self, shotResult, options):
        # The worker has already pulled this shot's ShotResult from the owning
        # device, so a target reduces it directly -- no getPersistenceManager()/
        # getShotResult() lookup needed.  A real target would fit or reduce
        # shotResult.getMeasurements() here.
        haveData = shotResult is not None

        return {
            "model": options.get("model", "none"),
            "haveShotData": haveData,
            "N": 1.0e6,
        }
