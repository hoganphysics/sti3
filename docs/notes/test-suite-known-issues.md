# Known pre-existing issues in the device test suite

Status as of 2026-07-13. All three issues below were verified to exist on **unmodified** code
(git stash A/B comparison against the working tree that added the measurement-grace changes),
so they are not regressions from the `waitForPlayAll` measurement-grace work.

Build/run recipe used for all reproductions (Windows, MSVC 14.36, Ninja tree at `build`):

```
cmd /c "\"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && cmake --build build --target sti3_test_device"
```

The test exe needs these on PATH or it dies at startup with 0xc0000135:
`build\src\device\src` (stidevice.dll), `build\src\network\src`,
the conda env root containing `python313.dll`, and its `Library\bin`.

---

## 1. Intermittent HANG: LocalLogManager network aggregation test wedges the whole suite

**Test:** `LocalLogManager network methods aggregate connected device logs`
(`test/device/locallogmanager_tests.cpp:416`, tags `[log] [network]`)

**Symptom:** The suite (or the test run in isolation) stops producing output forever. The process
stays alive with near-zero CPU (observed: 1.2 s CPU after 4 hours). The last stdout lines are the
device-collection listener prints:

```
++++ add( 127_0_0_1/44/RemoteDevice )
---- remove( 127_0_0_1/44/RemoteDevice )
```

i.e. the test body finished (the fake remote device was already removed) and the hang is in the
**teardown of the test's `LocalDevice`** ("LogDevice", module 4) — destructor-time thread joins.

**Frequency:** ~1/15 runs when running this test (paired with the following Logger test) in a loop
with a 60 s timeout; reproduced 2/30 on unmodified baseline code and 1/15 on a modified tree.
Because Catch2 runs tests in declaration order, a full-suite run that wedges with the add/remove
lines above as its last output is almost certainly this issue.

**Repro loop (PowerShell):**

```powershell
for ($i=0; $i -lt 30; $i++) {
  $p = Start-Process build\test\sti3_test_device.exe `
    -ArgumentList '"LocalLogManager network methods aggregate connected device logs"' `
    -PassThru -NoNewWindow
  if (-not $p.WaitForExit(60000)) { $p | Stop-Process -Force; Write-Output "run $i HUNG" }
}
```

**Debugging leads (not yet investigated):**
- The hang is during `LocalDevice` destruction. Candidate joins: `LocalEventEngineScheduler`
  dtor (`schedulerThread.join()`, `LocalEventEngineScheduler.cpp:236`), `~LocalEventEngine`
  (`resetPlayThread()` + `engineStateMessageGrouper.stop()`, `LocalEventEngine.cpp:159-164`),
  the log manager's flush/interval tasks (`TaskScheduler`), and the message dispatcher threads.
- The test exercises `getNetworkLogNames`/`getNetworkLogCount`/`getNetworkLogIDs` against a fake
  in-collection remote device; a log task or dispatcher callback still referencing the collection
  during dtor is plausible.
- To capture stacks: no cdb is installed on this machine; install Debugging Tools for Windows, or
  attach the VS debugger to the wedged PID (it sits there indefinitely, so there is plenty of time),
  and look for a thread blocked in a `join()`/condition wait during `~LocalDevice`.

## 2. FIXED (2026-07-13): flaky `findCompletedPlayJob` races in eventengine_missing_partner_tests

**Tests:** the three cancel-path tests in `test/device/eventengine_missing_partner_tests.cpp`
that asserted `REQUIRE(playJob != nullptr)` right after `waitForShotTerminal(...) == Canceled`
("Cannot Play Abstract Shot" ×2 and "Server cancels play when owned target loses parsed engine
before play").

**Cause:** a shot's `EngineJobStatus` turns `Canceled` slightly before the play job object is moved
into the scheduler's completed-jobs list (`completedPlayJobs`); the tests called
`findCompletedPlayJob` (single, immediate lookup) and lost the race ~7/10 runs.

**Fix:** switched those call sites to the file's own bounded-retry helper
`waitForCompletedPlayJob` (500 × 10 ms), which the neighboring tests already used. 10/10 after fix.
If the underlying ordering (status flips terminal before the job lands in the completed list) is
considered a real API contract violation, the place to look is
`LocalEventEngineScheduler::jobCanceled/jobComplete` vs. wherever `getStatus` reads job state.

## 3. Intermittent failure: rapid async write test

**Test:** `LocalDevice write handles rapid async SingleUndocumented shots without transient
directories` (`test/device/localdevice_write_tests.cpp:182`, failing CHECK at :189)

**Symptom/frequency:** fails ~1/10 runs in isolation. Name suggests it asserts that no transient
shot directories exist on disk while rapid asynchronous shots persist — a filesystem/async timing
race between the persistence writer and the test's directory scan. Not investigated further;
unrelated to the playback/timeout code.
