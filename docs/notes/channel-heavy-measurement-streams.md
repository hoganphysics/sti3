# Channel Heavy Measurement Stream Plan

Date: 2026-06-03

## Goal

Replace the current channel-measurement heavy-payload suppression with a lazy
stream reference mechanism.

The current `lastMeasurement` implementation avoids sending `BinaryData` and
`Image` values through channel snapshots and `ChannelUpdateMessage` by replacing
them with `MixedValueType::Empty`. This protects the DeviceMessage path and the
front end from large data transfers, but it also removes the ability for a
client to discover that heavy measurement data exists and pull it later.

The desired behavior is:

- Channel update messages stay lightweight.
- Remote clients can still see that a measurement exists and what type it is.
- Heavy `BinaryData` and binary-backed `Image` values can be pulled explicitly.
- Existing file-backed data continues to use the file-transfer path.
- CORBA, omniORB, and generated IDL details remain encapsulated in `stinetwork`.
- Code outside `stinetwork` uses abstract `include/sti` interfaces such as
  `BinaryData`, `BinaryDataStream`, `BinaryDataStreamTarget`, `FileHolder`,
  `FileHolderFactory`, `FileServer`, and `PersistenceManager`.

## Existing Pieces

The IDL already has a binary stream concept.

`src/network/idl/orbTypes.idl` defines:

```idl
enum TBinaryType {
    BinaryChar,
    BinaryUChar,
    BinaryUShort,
    BinaryShort,
    BinaryULong,
    BinaryLong,
    BinaryFloat,
    BinaryDouble,
    BinaryStream
};

interface TBinaryDataStream;

union TMixedBinaryData switch(TBinaryType)
{
    ...
    case BinaryStream: TBinaryDataStream data_stream;
};

struct TBinaryData
{
    short wordsize;
    TMixedBinaryData data;
};
```

`src/network/idl/deviceNet.idl` defines:

```idl
interface TBinaryDataStreamTarget
{
    void start();
    void writeNext(in TBinaryData data);
    void stop();
};

interface TBinaryDataStream
{
    void transfer(in TBinaryDataStreamTarget target);
};
```

The C++ wrapper classes already exist:

- `STI::Utils::BinaryDataStream`
- `STI::Utils::BinaryDataStreamTarget`
- `STI::Utils::LocalBinaryDataStream`
- `STI::Utils::LocalBinaryDataStreamTarget`
- `STI::Network::NetworkBinaryDataStream`
- `STI::Network::NetworkBinaryDataStreamTarget`
- `STI::Network::RemoteBinaryDataStream`
- `STI::Network::RemoteBinaryDataStreamTarget`

`NetworkConvert.cpp` already converts a large `BinaryData` payload to a
`TBinaryData` whose union branch is `BinaryStream` when
`bin->bytes() > omni::orbParameters::giopMaxMsgSize`.

On the receive side, `NetworkConvert.cpp` currently handles `BinaryStream` by
immediately pulling the remote stream into local `BinaryData`. This is useful
for normal RPC return values, but it does not solve the channel update problem:
message delivery still performs the heavy transfer eagerly.

## Current Heavy-Payload Guard

The last-measurement implementation currently uses
`makeLightweightChannelMeasurementValue()` from `include/sti/device/ChannelState.h`.

It treats these as heavy:

- `MixedValueType::Binary`
- `MixedValueType::Image`
- vectors containing either of the above

For heavy values it returns `MixedValueType::Empty`. This is safe but lossy.

The two primary call sites are:

- `LocalChannelManager::handleChannelMeasurementRefreshEvent()`
- `Convert_Channel.cpp` when building `TChannel.lastMeasurement`

The goal of this plan is to replace those call sites with a stream-backed
lightweight value, not a data-dropping value.

## Desired Semantics

For channel measurement pushes and snapshots:

- `File` values remain lightweight file references.
- `Image` values backed by `FileHolder` or `FileID` remain file-backed image
  references.
- `BinaryData` values become stream-backed `MixedValueType::Binary`.
- `Image` values backed by `BinaryData` become stream-backed
  `MixedValueType::Image`.
- The receiver does not pull the binary bytes during message conversion.
- The receiver stores a lazy object in the remote channel cache.
- The front end can explicitly pull or save the data when the user asks.

For normal read/RPC return paths:

- The existing eager behavior may remain the default.
- A large returned `BinaryData` can still stream internally to avoid GIOP max
  message size, then materialize before returning to the caller.

This implies conversion needs a policy:

- Eager conversion: convert a stream to local data immediately.
- Lazy conversion: attach the remote stream reference to `BinaryData` and return
  without transferring.

## Proposed Public Abstractions

### BinaryData

Extend `STI::Utils::BinaryData` so it can represent either local bytes or a
lazy stream reference.

Suggested API:

```cpp
class BinaryData
{
public:
    bool hasLocalData() const;
    bool hasStream() const;
    bool isMaterialized() const;

    size_t length() const;
    size_t bytes() const;
    size_t wordsize() const;

    void attachStream(const std::shared_ptr<BinaryDataStream>& stream);
    void attachStream(const std::shared_ptr<BinaryDataStream>& stream,
                      size_t length,
                      size_t wordsize);

    bool materialize();
    bool transferTo(const std::shared_ptr<BinaryDataStreamTarget>& target);
};
```

`materialize()` should be idempotent:

- If local data is already present, return true.
- If no local data but a stream exists, pull from the stream and store the data.
- If neither local data nor a stream exists, return false.

`transferTo()` should allow a caller to copy the data into a target without
necessarily materializing inside this `BinaryData` instance first.

`BinaryData::attachStream()` already exists, but it only stores the stream
privately and there is no public pull/query API. The implementation can build
on that existing storage.

### Stream Metadata

`TBinaryData` already contains `short wordsize`. The stream path must preserve
and consume it. Add tests that prove `wordsize` is populated for
`BinaryStream` and available on the receiver before the stream is pulled.

The front end will likely also need to display size before pulling. `wordsize`
alone is not enough for this. Consider extending `TBinaryData` with one or both
of:

```idl
unsigned long length;
unsigned long bytes;
```

Recommended IDL shape:

```idl
struct TBinaryData
{
    short wordsize;
    unsigned long length;
    unsigned long bytes;
    TMixedBinaryData data;
};
```

If avoiding an IDL change is preferred initially, at least preserve `wordsize`
and use `bytes() == 0` or unknown on lazy receivers until the data is pulled.
However, a total byte count is useful for UI decisions and download progress.

### Byte Chunking

It is acceptable for stream chunks to be byte chunks (`char*`) even when the
original `BinaryData` was typed. The channel/front-end use case mostly needs a
byte payload that can be saved or decoded.

The plan should not require fully typed chunk transport. Keep small inline
typed conversions as they are today.

Still, fix the stream path so it works for any `BinaryData` that has raw bytes:

- `LocalBinaryDataStream::transfer()` should split by bytes using
  `BinaryData::getBytes()`.
- Chunks can be `char*` `BinaryData`.
- The stream envelope keeps the original `wordsize`.
- A materialized lazy payload can be byte-addressable, with `wordsize` retained
  as metadata.

If future C++ callers need typed array reconstruction after streaming, add a
separate typed reconstruction API later.

## Network Conversion Design

### Do Not Change All Conversions At Once

Avoid globally changing `convert<TBinaryData, BinaryData>` to lazy behavior,
because some RPC paths expect the return value to be immediately available.

Instead, add explicit conversion helpers inside `stinetwork`, for example:

```cpp
enum class BinaryPayloadPolicy
{
    InlineOrEagerStream,
    PreferStreamReference,
    PreserveStreamReference
};
```

Then add helper functions in `NetworkConvert` or a channel-state conversion
module:

```cpp
bool convertBinaryData(
    const std::shared_ptr<STI::Utils::BinaryData>& bin,
    STI::TNetwork::TBinaryData& tBin,
    BinaryPayloadPolicy policy);

bool convertBinaryData(
    const STI::TNetwork::TBinaryData& tBin,
    std::shared_ptr<STI::Utils::BinaryData>& bin,
    BinaryPayloadPolicy policy);

bool convertMixedValue(
    const STI::Utils::MixedValue& value,
    STI::TNetwork::TMixedValue& tValue,
    BinaryPayloadPolicy policy);

bool convertMixedValue(
    const STI::TNetwork::TMixedValue& tValue,
    STI::Utils::MixedValue& value,
    BinaryPayloadPolicy policy);
```

Existing `convert<TBinaryData, BinaryData>` can remain eager. The channel
message and channel snapshot conversions should use the lazy/preserve policy.

### Sending Heavy Channel State

For `ChannelUpdateMessage.measurementValues` and `TChannel.lastMeasurement`,
use `PreferStreamReference`.

Behavior:

- For `MixedValueType::Binary`, send `TBinaryData(BinaryStream)` regardless of
  current byte size. This keeps message delivery lightweight even for values
  below GIOP max size.
- For `MixedValueType::Image`, preserve image metadata and file ID. If it has
  binary image data, send `ImageDataBinary` containing stream-backed
  `TBinaryData`.
- For vectors, recursively stream any binary/image children instead of replacing
  the whole vector with `Empty`.
- For `File`, continue sending the existing file reference.

The existing `NetworkBinaryDataStream` creation pattern should be reused:

```cpp
auto networkDataStream =
    std::make_shared<NetworkBinaryDataStream>(bin.get(), chunkSize);
bin->attachStream(networkDataStream);
tBin.data.data_stream(tDataStream);
```

`bin->attachStream(networkDataStream)` is important because it keeps the servant
alive as long as the source `BinaryData` is alive. Since `lastMeasurement` owns
the `MixedValue`, this gives a natural lifetime: the stream remains valid until
the device replaces or clears that measurement, or the device process exits.

### Receiving Heavy Channel State

For channel update messages and channel snapshots, use `PreserveStreamReference`.

Behavior for `TBinaryData(BinaryStream)`:

- Create `std::shared_ptr<RemoteBinaryDataStream>`.
- Create a `BinaryData` instance.
- Attach the stream to `BinaryData`.
- Set `length`, `bytes`, and `wordsize` metadata from `TBinaryData`.
- Do not call `transfer()` during conversion.
- Return `MixedValueType::Binary` or an `Image` with stream-backed
  `BinaryData`.

The current eager branch in `convert<TBinaryData, BinaryData>` has a bug:

```cpp
case TBinaryType::BinaryStream:
    ...
    remoteDataStream->transfer(networkTarget);
    // success is never set true
```

Fix this while implementing the stream work. In eager mode, set `success` based
on whether the transfer completed and local data exists. In lazy mode, set
`success = true` after attaching a valid stream reference.

## Channel Message Changes

Replace the current lightweight guard in:

- `LocalChannelManager::handleChannelMeasurementRefreshEvent()`
- `Convert_Channel.cpp` for `TChannel.lastMeasurement`

with a channel-state conversion policy.

Do not make `stidevice` depend on `stinetwork`.

Recommended approach:

- Keep `LocalChannelManager` sending the real `MixedValue` in local in-process
  messages.
- For remote message forwarding, apply lazy stream conversion in
  `Convert_DeviceMessage.cpp`, where `stinetwork` already owns IDL/CORBA
  details.
- For `TChannel` snapshots, apply lazy stream conversion in
  `Convert_Channel.cpp`.

This means local listeners in the same process may still see the real
`BinaryData` pointer. Remote listeners see a stream-backed `BinaryData`.

If local in-process frontends also need protection from large data, add an
optional local message policy later. The initial scope should solve remote
transport without leaking network dependencies into `stidevice`.

## Remote Channel Cache

`RemoteChannelManager::ChannelDataTuple` already has:

```cpp
STI::Utils::MixedValue measurement;
```

Store the lazy `MixedValue` directly in this field.

`RemoteChannel::getLastMeasurement()` can continue returning `MixedValue`. The
returned value may contain:

- local scalar data,
- a `FileID`,
- a lazy `BinaryData`,
- an `Image` with lazy binary data.

No additional remote-channel public API is required at the C++ channel level if
`BinaryData` and `Image` expose pull/save operations.

## Results Collection Semantics

`LocalResultsCollector::addMeasurements()` is the end-of-shot archival boundary.
It is different from the live channel-update path.

At the end of a shot, the server-side results collector gathers measurements
from devices in the shot. Measurements can arrive from remote devices through
`RemoteResultsCollector` / `TResultsCollector_i`, where `TMeasurement` values
are converted back into local `Measurement` objects before
`LocalResultsCollector::addMeasurements()` is called.

Current `LocalResultsCollector` behavior:

- `addMeasurements()` copies the measurement vector into the shot result.
- It then extracts each measurement result into a `MixedValue`.
- `transferValue()` recursively processes vectors.
- `File` values are transferred from the device's `sourceFileServer` to the
  server and rewritten as a local `FileID`.
- `Image` values call `Image::write(sourceFileServer, localFileHandle)`.
  Existing `Image::write()` handles file-backed images through the file server
  and local binary-backed images by writing bytes to the destination holder.
- `Binary` values are written to a server-local
  `binary_measurement*.bin` file and rewritten as a local `FileID`.

The stream plan must preserve this archival behavior. Lazy binary/image
references are acceptable in live channel state, but they should not be stored
as lazy remote references inside completed shot results. Completed shot results
should contain server-local file references where possible.

Required behavior after lazy receive is added:

- If `transferValue()` sees `MixedValueType::Binary` whose `BinaryData` is
  lazy/stream-backed, it must pull the stream during result collection and write
  the bytes into the server-local binary measurement file.
- If `transferValue()` sees `MixedValueType::Image` whose `Image` contains
  lazy/stream-backed `BinaryData`, result collection must pull that data and
  write the image to the server-local destination file, then update the image to
  reference the local file holder.
- File-backed images should continue using the existing file-server transfer
  path.
- After successful transfer, the stored measurement value should be rewritten to
  a local `FileID` or an `Image` whose file holder/file ID points at the
  server-local copy. It should not remain a remote stream reference.
- Shared references should still be cached so the same file, image, or binary
  object is transferred only once per `addMeasurements()` call.

Implementation options for binary data:

- Simple first pass: call `BinaryData::materialize()` before existing
  `getBytes()`/`FileHolder::write()` logic. This may use server memory equal to
  the payload size, but it is straightforward and preserves existing
  `transferValue()` structure.
- Better follow-up: add a `BinaryDataStreamTarget` implementation that writes
  chunks directly into a `FileHolder`. Then `transferValue()` can stream lazy
  binary data into the results file without materializing the full payload in
  memory.

`Image::write()` should also be updated or audited so that a binary-backed
image with lazy `BinaryData` either materializes or streams its data before
calling `getBytes()`. This keeps image transfer behavior correct whether the
image was produced locally or received lazily over the network.

Do not make `LocalResultsCollector` depend on `stinetwork`. It should only use
`BinaryData`, `BinaryDataStream`, `BinaryDataStreamTarget`, `FileHolder`, and
`FileServer` abstractions from `include/sti`.

## Python / stidevicepy Design

The front end uses the STI API through `stidevicepy`, so the lazy behavior must
be visible and controllable from Python.

Current behavior:

- `MixedValue.getValue()` converts `MixedValueType::Binary` to Python `bytes`.
- `MixedValue.getValue()` does not currently return useful data for
  `MixedValueType::Image`.

With lazy binary references, `getValue()` should not accidentally pull large
remote data unless this backwards-compatible behavior is explicitly desired.

Recommended additions:

- Bind `BinaryData` as a Python class.
- Bind enough `Image` API for image measurements.
- Add explicit pull/save APIs.

Example Python API:

```python
value = channel.getLastMeasurement()

if value.getType() == stidevicepy.MixedValueType.Binary:
    binary = value.getBinary()
    print(binary.bytes(), binary.wordsize(), binary.isMaterialized())
    payload = binary.getBytes()      # explicit pull if lazy
    binary.save("/tmp/measurement.bin")

if value.getType() == stidevicepy.MixedValueType.Image:
    image = value.getImage()
    print(image.getWidth(), image.getHeight(), image.getFileID())
    image.save("/tmp/measurement.raw")
```

Possible `BinaryData` Python methods:

- `bytes()`
- `length()`
- `wordsize()`
- `isMaterialized()`
- `hasStream()`
- `pull()`
- `getBytes()`
- `save(path)`

Possible `Image` Python methods:

- `getFileID()`
- `getHeight()`
- `getWidth()`
- `hasData()`
- `hasFile()`
- `getData()`
- `pullData()`
- `save(path)`

For backwards compatibility, `MixedValue.getValue()` may continue returning
`bytes` for binary data, but document that this can trigger a pull. Prefer the
explicit `getBinary()` API in frontend code.

## Lifetime And Failure Semantics

The stream reference is only valid while the source object remains alive on the
device process that produced it.

Initial lifetime rule:

- The stream remains valid while the source channel's `lastMeasurement` still
  refers to that `BinaryData` or `Image`.
- Replacing `lastMeasurement` may invalidate older stream references.
- Device shutdown or network disconnect invalidates the stream.

Suggested failure behavior:

- `BinaryData::materialize()` returns false on stale stream, nil reference, or
  transfer exception.
- Python `getBytes()` can raise an exception or return `None`; choose one
  consistent with existing stidevicepy style.
- Frontend code should treat lazy payload download as best effort.

Later improvement:

- Add a small stream token or measurement generation ID if clients need to
  detect that a referenced measurement has been replaced.

## IDL Impact

Required or recommended IDL changes:

- Verify `TBinaryData.wordsize` is populated and read for `BinaryStream`.
- Add `length` and/or `bytes` to `TBinaryData` if pre-pull size metadata is
  needed.

After editing IDL:

```bash
cd src/network
./compileIDL.sh
```

This is a wire-ABI change. Regenerated stubs must be committed with the IDL
change.

## Implementation Todo

- [ ] Add tests around the current stream conversion bug.
  - Create or force a `TBinaryData(BinaryStream)`.
  - Confirm eager conversion returns `true`.
  - Confirm eager conversion materializes data.

- [ ] Fix `convert<TBinaryData, std::shared_ptr<BinaryData>>()`.
  - Set `success = true` for successful `BinaryStream` conversion.
  - Preserve `wordsize` for stream-backed data.
  - Add focused tests.

- [ ] Decide the exact stream metadata fields.
  - Keep and use existing `TBinaryData.wordsize`.
  - Add `length` and/or `bytes` to `TBinaryData` if the front end should show
    size before pulling.
  - Regenerate IDL stubs if fields are added.

- [ ] Extend `BinaryData`.
  - Add query methods for local data and stream availability.
  - Add lazy stream attachment with `length` and `wordsize` metadata.
  - Add `materialize()` or `pull()` method.
  - Add `transferTo(BinaryDataStreamTarget)` method.
  - Keep `stinetwork` types out of `include/sti/utils`.

- [ ] Make byte chunking robust.
  - Ensure `LocalBinaryDataStream` can stream any `BinaryData` via `getBytes()`.
  - It is acceptable for chunks to be byte chunks.
  - Preserve original `wordsize` as metadata.
  - Add tests for non-char source data if preserving typed data matters.

- [ ] Add explicit conversion policy in `stinetwork`.
  - Keep existing eager conversion for normal RPC paths.
  - Add lazy conversion for channel snapshots and channel update messages.
  - Avoid changing all `convert<TMixedValue, MixedValue>` behavior globally.

- [ ] Update channel update conversion.
  - Convert `ChannelUpdateMessage.measurementValues` with lazy heavy payloads.
  - Preserve vectors containing binary/image children.
  - Stop replacing heavy measurement values with `Empty` on remote message
    conversion.

- [ ] Update channel snapshot conversion.
  - Convert `TChannel.lastMeasurement` with lazy heavy payloads.
  - Stop replacing heavy snapshot measurement values with `Empty`.

- [ ] Revisit local message behavior.
  - Local in-process listeners can initially receive the real `MixedValue`.
  - If this is too heavy for local frontends, add a non-network lazy local
    reference policy later.

- [ ] Extend `Image` behavior.
  - For file-backed images, continue using the existing file path/reference.
  - For binary-backed images, use stream-backed `TBinaryData`.
  - Add pull/save helpers if not already available.
  - Ensure `Image::write()` handles lazy/stream-backed binary image data by
    materializing or streaming it before writing to a destination `FileHolder`.
  - Preserve image metadata, width, height, and file ID.

- [ ] Update `LocalResultsCollector` result archival.
  - Treat result collection as an eager pull boundary for lazy heavy payloads.
  - In `transferValue()`, materialize or stream lazy `BinaryData` before writing
    the server-local `binary_measurement*.bin` file.
  - In `transferValue()`, ensure lazy binary-backed `Image` values are pulled
    and written to the server-local image file.
  - Rewrite successfully transferred binary/image measurements to server-local
    file references, not remote stream references.
  - Preserve existing caching so shared file/image/binary references are
    transferred once per `addMeasurements()` call.
  - Keep the implementation in terms of abstract `include/sti` interfaces, not
    `stinetwork` classes.

- [ ] Extend stidevicepy.
  - Bind `BinaryData` or add `MixedValue.getBinary()`.
  - Bind `Image` or add `MixedValue.getImage()`.
  - Add explicit pull/save methods for heavy data.
  - Document that `MixedValue.getValue()` may pull binary data if kept for
    backwards compatibility.

- [ ] Add focused tests.
  - `BinaryData` lazy stream attachment and materialization.
  - `LocalBinaryDataStream` byte chunking.
  - `NetworkConvert` eager `BinaryStream` conversion.
  - `NetworkConvert` lazy `BinaryStream` conversion.
  - `MixedValue(Binary)` lazy channel update round trip.
  - `MixedValue(Image)` lazy channel update round trip.
  - `TChannel.lastMeasurement` lazy snapshot round trip.
  - Remote channel cache stores lazy measurements.
  - `LocalResultsCollector::addMeasurements()` pulls lazy `BinaryData` and
    stores a server-local `FileID`.
  - `LocalResultsCollector::addMeasurements()` pulls lazy binary-backed `Image`
    data and stores a server-local image/file reference.
  - Result collection does not leave lazy remote stream references in completed
    `ShotResult` measurements.
  - stidevicepy can inspect and explicitly pull a lazy binary measurement.

- [ ] Update docs and frontend handoff.
  - Explain that `lastMeasurement` may contain lazy heavy payloads.
  - Explain how Python clients detect and pull binary/image data.
  - Explain stream lifetime: valid until replaced, cleared, device exits, or
    network reference dies.

## Suggested First Implementation Pass

1. Add focused tests that expose the existing `BinaryStream` conversion bug.
2. Fix eager `BinaryStream` conversion and preserve `wordsize`.
3. Add `BinaryData` lazy-stream query and materialization API.
4. Add a lazy conversion policy inside `stinetwork`.
5. Use the lazy policy only for `TChannel.lastMeasurement` and
   `ChannelUpdateMessage.measurementValues`.
6. Update `LocalResultsCollector::transferValue()` and `Image::write()` so
   server result collection pulls lazy binary/image measurements into
   server-local files.
7. Add Python wrappers for explicit binary/image pull.
8. Remove or bypass `makeLightweightChannelMeasurementValue()` only after lazy
   channel update and snapshot tests pass.

## Open Questions

- Should `TBinaryData` add both `length` and `bytes`, or is one enough?
- Should `MixedValue.getValue()` in Python preserve backwards-compatible eager
  binary conversion, or should it return a `BinaryData` wrapper for binary
  values?
- Should local in-process message listeners receive the original heavy value or
  a local lazy reference?
- Do any C++ callers rely on typed binary array reconstruction after network
  streaming, or are byte chunks sufficient for all practical heavy measurement
  consumers?
- Should stale stream pulls return false, throw, or set an error object/message
  for frontend display?
