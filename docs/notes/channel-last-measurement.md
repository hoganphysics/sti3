# Channel Last Measurement Plan

Date: 2026-06-02

## Goal

Add persistent channel state for both:

- `lastValue`: the most recent command/configuration value sent to a channel.
- `lastMeasurement`: the most recent measurement data produced by a channel.

This should let input channels keep both sides of their state:

- the read argument or measurement configuration in `lastValue`
- the returned measurement data in `lastMeasurement`

Output channels should continue to use `lastValue` for the most recent write. Their `lastMeasurement` should stay empty.

The state must be visible through local channel objects, remote channel objects, grouped channel update messages, and the `stidevicepy` API so frontend clients can reconnect and inspect the current channel state.

`lastMeasurement` only needs to persist for the lifetime of the running device process. It does not need to survive process restart.

## Current Findings

`Channel` currently exposes only:

- `saveLastValue(const MixedValue&)`
- `getLastValue()`

`LocalChannel` stores only one `MixedValue lastValue`. It has commented-out notes for a possible `lastInValue`, but no implemented second value.

`LocalChannelManager::writeChannel()` currently calls `localDevice->write(...)` and then saves the written value into `lastValue`.

`LocalChannelManager::readChannel()` currently calls `localDevice->read(...)` and then saves the returned data into `lastValue`. This is the main explicit-read path where input-channel measurement data overwrites the read argument/configuration state.

There is another important path: `DevicePy.read()` calls `Device::read()` directly, not `ChannelManager::readChannel()`. For a local device, `LocalDevice::read()` currently invokes the virtual `readChannel()` implementation but does not update any channel cache. For a remote device, `RemoteDevice::read()` does go through `RemoteChannelManager`.

Event playback has its own behavior. `LocalEventEngine::updateChannelValues()` saves raw event values into `lastValue`, including measurement event values. Actual measurement results are stored in `Measurement` objects and shot results, but they are not copied into channel state.

Network transport currently exposes one cached channel value:

- `TChannel.lastValue` in `src/network/idl/orbTypes.idl`
- `TChannelUpdateMessage.channelValues` for grouped channel value updates
- `RemoteChannelManager::ChannelDataTuple::value`

`stidevicepy` currently exposes only:

- `Channel.getLastValue()`
- `ChannelUpdateMessage.channelValues()`

## Proposed Semantics

`lastValue` should mean the output-side value for every channel:

- Output channel: value passed to `write()`.
- Input channel with non-empty `outputType`: value passed to `read()` as a measurement command/configuration argument.
- Input channel with `outputType == MixedValueType::Empty`: remain `MixedValueType::Empty`; do not rewrite `lastValue` on every read because there is no valid changing output-side value.
- Event-engine measurement: raw measurement event value, when the input channel has a non-empty `outputType`.

`lastMeasurement` should mean input-side measurement data:

- Explicit input read: the `data` returned by `readChannel()`.
- Event-engine measurement: the data stored in the relevant `Measurement` after `collectData()`.
- Output channel: remain `MixedValueType::Empty`; do not update `lastMeasurement`.

Do not add a new `ChannelUpdateMessageType` for measurement updates. Keep `ChannelUpdateMessageType::ChannelValue` as the grouped state-update type and add a second map:

```cpp
std::map<short, STI::Utils::MixedValue> measurementValues;
```

A grouped `ChannelValue` message may contain updates in `channelValues`, `measurementValues`, or both.

## Implementation Todo

- [ ] Extend `Channel`.
  - Add pure virtual `saveLastMeasurement(const STI::Utils::MixedValue& value)`.
  - Add pure virtual `const STI::Utils::MixedValue getLastMeasurement() const`.
  - Keep existing `lastValue` APIs for compatibility.

- [ ] Extend `LocalChannel`.
  - Add `STI::Utils::MixedValue lastMeasurement`.
  - Implement `saveLastMeasurement()` and `getLastMeasurement()`.
  - Add a template convenience overload mirroring `saveLastValue<T>()` if useful.
  - Fire a distinct internal refresh callback when `lastMeasurement` changes.

- [ ] Extend `ChannelRefreshListener`.
  - Add `handleChannelMeasurementRefreshEvent(short channelNumber, const MixedValue& value)`.
  - Update all implementations and test helpers.

- [ ] Extend `ChannelUpdateMessage`.
  - Add `measurementValues`.
  - Update `appendMessage()` to merge both `channelValues` and `measurementValues`.
  - Keep `groupable()` true for `ChannelValue` messages.
  - Add a clear construction path for measurement-only updates, for example a named static factory or a tagged constructor, so the existing `(trace, channel, value)` constructor remains a last-value update.

- [ ] Update local read/write state handling.
  - Ensure successful writes save the write value to `lastValue`.
  - Ensure successful input reads save the read argument/configuration to `lastValue` only when the channel's `outputType` is not `MixedValueType::Empty`.
  - Ensure successful input reads save returned data to `lastMeasurement`.
  - Never update `lastMeasurement` for output channels.
  - Include direct `LocalDevice::read()` / `LocalDevice::write()` callers, not only `LocalChannelManager`, so C++ and Python convenience calls update state consistently.
  - Validate read return data against `Channel::getInputType()` before saving it as `lastMeasurement`.
  - On invalid read return data, do not update cached state. Direct/ad-hoc reads should fail. Reads occurring during async play should produce an `EnginePlayingMessage` error so the frontend can retrieve the problem from the async engine result.
  - Avoid double-saving from `LocalChannelManager` once `LocalDevice` owns the direct cache update, or accept grouped duplicate updates only where event playback intentionally replays raw event state.

- [ ] Update event-engine measurement state.
  - Keep `LocalEventEngine::updateChannelValues()` responsible for raw event values in `lastValue`.
  - Skip raw measurement event `lastValue` updates when the input channel has `outputType == MixedValueType::Empty`.
  - After `SynchronousEvent::collectData()`, iterate ready `Measurement` objects and save their data into the target channel's `lastMeasurement`.
  - Validate each collected measurement against the channel's `inputType` before saving.
  - For invalid collected data, add an `EnginePlayingMessage` error through the measurement/play-message path. `SynchronousEvent::measureMessages` and `LocalEventEngine::measureData()` already provide the path that is appended into async engine results.
  - Confirm behavior for file/image/binary measurements, where `MixedValue` may hold file-oriented data.

- [ ] Extend network IDL and generated stubs.
  - Add `TMixedValue lastMeasurement` to `TChannel`.
  - Add `TChannelUpdateTupleSeq measurementValues` to `TChannelUpdateMessage`.
  - Run `src/network/compileIDL.sh` after editing IDL.
  - Treat this as a wire-ABI change: old clients and servers will not understand the new struct layout.

- [ ] Update network conversion.
  - `Convert_Channel.cpp`: convert `Channel::getLastMeasurement()` into `TChannel.lastMeasurement`.
  - `Convert_Channel.cpp`: pass `TChannel.lastMeasurement` into `RemoteChannel`.
  - `Convert_DeviceMessage.cpp`: convert `measurementValues` both directions alongside `channelValues`.
  - Add network conversion tests for `TChannel` and `TChannelUpdateMessage` round trips with both maps populated.

- [ ] Protect large measurement payloads on push paths.
  - Audit `MixedValue` behavior for `Image`, `BinaryData`, and `File`/`FileHolder`-style results before pushing `lastMeasurement` over `DeviceMessage` or channel snapshots.
  - Prefer sending a lightweight reference/handle that can be transferred on demand through existing persistence/file-transfer APIs.
  - If a heavy type cannot be represented as a lightweight reference yet, add a temporary guard that suppresses or replaces the pushed `lastMeasurement` payload with an empty/error/reference value rather than saturating channel update infrastructure.
  - Ensure channel snapshots over `TChannel` follow the same protection rules as push messages.

- [ ] Extend remote channel cache.
  - Add `measurement` to `RemoteChannelManager::ChannelDataTuple`.
  - Add `RemoteChannelManager::getLastMeasurement(short)`.
  - Add `RemoteChannel::getLastMeasurement()`.
  - Update `RemoteChannelManager::handleMessage()` so `channelValues` updates `value` and `measurementValues` updates `measurement`.
  - Update initial remote channel snapshots from `TChannel.lastMeasurement`.

- [ ] Extend `stidevicepy`.
  - Add `Channel.getLastMeasurement()` in `Channel_wrap.cpp`.
  - Add `ChannelUpdateMessage.measurementValues()` in `DeviceMessage_wrap.cpp`.
  - Consider updating `DevicePy.write()` and `DevicePy.read()` to use the channel manager consistently, or rely on the `LocalDevice` cache update once implemented.
  - Add Python-facing tests or integration coverage for local and remote channels.

- [ ] Consider Java/SWIG follow-up.
  - `src/stijava/src/swig/device.i` includes `Channel.h` and `DeviceMessage.h`, so regenerated Java bindings will need the new virtual methods and `measurementValues` map.
  - This is probably secondary to `stidevicepy`, but it should not be forgotten if Java remains supported.

- [ ] Update docs and examples.
  - Document the distinct meanings of `lastValue` and `lastMeasurement`.
  - Update channel listener examples to show both `channelValues()` and `measurementValues()`.
  - Fix any docs that imply input-channel `lastValue` is measurement data.
  - Document that input channels with empty `outputType` intentionally keep `lastValue == Empty`.

## Tests To Add Or Update

- [ ] `test/device/channel_tests.cpp`: `Channel` test double supports `lastMeasurement`.
- [ ] `test/device/localchannelmanager_tests.cpp`: explicit input read keeps read argument in `lastValue` and returned data in `lastMeasurement`.
- [ ] `test/device/localchannelmanager_tests.cpp`: input reads with `outputType == MixedValueType::Empty` update `lastMeasurement` but keep `lastValue` empty.
- [ ] `test/device/localchannelmanager_tests.cpp`: output writes update `lastValue` but keep `lastMeasurement` empty.
- [ ] Add or update local device tests so `LocalDevice::read()` and `DevicePy.read()` paths update channel state.
- [ ] Add local device tests showing wrong returned measurement type fails, does not update `lastMeasurement`, and produces an `EnginePlayingMessage` error during async play.
- [ ] Add event-engine coverage showing a measurement event updates `lastValue` with the event argument and `lastMeasurement` with collected data.
- [ ] Add event-engine coverage showing a measurement event with empty `outputType` leaves `lastValue` empty.
- [ ] `test/network/convert/*`: round-trip `TChannel.lastMeasurement`.
- [ ] `test/network/convert/*`: round-trip `TChannelUpdateMessage.measurementValues`.
- [ ] Add remote channel manager or integration coverage proving a remote client sees both initial snapshots and pushed updates.
- [ ] Add `stidevicepy` coverage for `Channel.getLastMeasurement()` and `ChannelUpdateMessage.measurementValues()`.
- [ ] Add coverage for large/heavy measurement types so channel update messages and snapshots do not push full payloads.

## Profile Semantics

Channel profiles should be able to apply to input channels now that `lastValue` represents the input channel's output-side configuration. The profile path should no longer exclude a channel just because it is an input channel; input channels with meaningful command/configuration values can be symmetric with output channels. This also supports existing usage patterns where users model simple hardware operations with input channels, for example channels with simple `bool` return data.

Profile save/load should therefore be updated carefully:

- Save `lastValue` for output channels.
- Save `lastValue` for input channels when `outputType != MixedValueType::Empty`.
- Input channels with `outputType == MixedValueType::Empty` have no meaningful profile value; omitting them or saving `Empty` should be treated as a no-op.
- Do not save `lastMeasurement`; measurement data is runtime/result state, not profile state.
- Loading an input-channel profile value should restore the cached/configuration `lastValue` without pretending a measurement occurred. The load path may need a channel-manager helper distinct from `writeChannel()`, because `writeChannel()` is output-only today.

## Open Questions

- What is the exact helper/API for applying profile values to input-channel `lastValue` without invoking `writeChannel()`?
- Which `MixedValue` heavy types are already safe lightweight references over network conversion, and which need temporary push suppression?

## Recommended First Pass

Implement the in-memory/device-lifetime state first:

1. Add `lastMeasurement` to `Channel`, `LocalChannel`, `RemoteChannel`, update messages, IDL, conversion, and `stidevicepy`.
2. Make explicit reads and direct `Device::read()` convenience calls update both cached fields.
3. Add event-engine measurement-result propagation into `lastMeasurement`.
4. Add input-channel `lastValue` support to profiles, but leave `lastMeasurement` out of profile and disk persistence.
5. Add large-payload protection before enabling pushed `lastMeasurement` updates for heavy `MixedValue` types.
