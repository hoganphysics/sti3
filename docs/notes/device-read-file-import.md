# Device Read File Import Plan

Date: 2026-06-19

## Goal

Allow Python and C++ callers to pass file-backed arguments to `device.read()`
when the file originates outside the target device.

The first version should use an eager import step:

```python
imported = device.getPersistenceManager().importFile(source_id, source_server, options)
result = device.read(channel, imported.fileID)
imported.close()
```

The target device should receive a normal `MixedValueType::File` value whose
`FileID` resolves through the target device's own persistence/file-server
namespace. The `read()` API should not need to carry a `FileServer` reference.

## Current Problem

`MixedValueType::File` stores only a `FileID`. It does not store a
`FileHolder`, `FileServer`, or other capability reference.

This is sufficient in the `ResultsCollector` pathway because
`ResultsCollector::addMeasurements()` receives two pieces of information:

- measurement values containing `FileID`
- a separate `sourceFileServer` that can resolve those IDs

`device.read(channel, value)` currently has only the value. If `value` is a
`FileID` that refers to a caller-owned virtual file, the target device has no
reliable way to know which file server owns it.

## First-Version Design

Add an eager file-import operation to `PersistenceManager`.

Conceptually:

```cpp
struct ImportFileOptions
{
    ImportStorage storage = ImportStorage::DiskTemporary;
    ImportCollisionPolicy collision = ImportCollisionPolicy::Unique;
    ImportLifetime lifetime = ImportLifetime::Handle;
    std::chrono::seconds ttl = std::chrono::minutes(10);
};

class ImportedFile
{
public:
    STI::Utils::FileID getFileID() const;
    bool close();
};

std::shared_ptr<ImportedFile> PersistenceManager::importFile(
    const STI::Utils::FileID& sourceID,
    const std::shared_ptr<STI::Utils::FileServer>& sourceServer,
    const ImportFileOptions& options);
```

`importFile()` should perform the transfer immediately:

1. Choose a target-local destination `FileID`.
2. Create a target-owned destination `FileHolder`.
3. Transfer `sourceID` from `sourceServer` into that destination.
4. Register the destination with the target device's file server when needed.
5. Return an `ImportedFile` handle whose `FileID` is safe to pass to
   `device.read()`.

After import, the caller passes only the imported target `FileID` to `read()`.
The target device resolves that ID through its own persistence manager/file
server.

## Storage Options

`ImportStorage::DiskTemporary` should be the default. It avoids unbounded
in-memory growth for ordinary callers and is safer for image-sized payloads.
`ImportStorage::Virtual` should remain available for callers that explicitly
want memory-backed transfer.

### `ImportStorage::DiskTemporary`

Use a temporary disk-backed `FileHolder` under the temporary directory provided
by the target persistence manager.

The implementation should use the same path source as
`PersistenceManager::getTemporaryPath()` rather than inventing a separate import
directory. If a subdirectory is needed, create it below that temporary path, for
example:

```text
<persistence temporary path>/imports/
```

The destination should also be created through the configured
`FileHolderFactory`. This keeps remote callback behavior abstract and avoids
requiring non-network code to include `stinetwork` headers.

Disk temporary imports should also be tracked by the import handle. Releasing
the handle should unregister the target `FileID` and remove the temporary file
when possible.

### `ImportStorage::Virtual`

Use a `VirtualFileHolder` as the target destination.

The holder should be created through the target persistence manager's configured
`FileHolderFactory`, not by direct construction in caller-facing code.

This matters for remote transfers. When a device is network-exposed,
`NetworkDevice` configures a `NetworkFileHolderFactory`; the returned holder can
be exposed as a `TFileHolder` destination so a remote source `FileServer` can
write into it.

Virtual imports must be registered with the target file server. The current
`VirtualFileServer` has no lifetime policy, so the import handle must own the
registration and remove it on release.

## Lifetime Model

Do not make callers manually call `FileServer::deleteFile()` as the primary
cleanup API. That is too easy to leak and exposes a low-level mechanism that
does not express ownership.

Use handle-scoped lifetime by default:

```python
with persistence.importFile(source_id, source_server, options) as imported:
    result = device.read(channel, imported.fileID)
```

The `ImportedFile` handle should release the import when:

- `ImportedFile.close()` is called
- a Python context manager exits
- the C++ shared handle is destroyed, as a best-effort fallback

The target persistence manager should also maintain a safety cleanup policy:

- TTL for unreleased transient imports
- optional maximum total imported bytes
- optional maximum imported file count

Cleanup should evict only released or expired imports. It should not delete an
active import simply because a quota fills up. If there is no safe eviction
candidate, a new import should fail.

## Import Registry

The target persistence manager should own an import registry. Each entry should
track:

- import ID, distinct from `FileID`
- target `FileID`
- storage type
- source `FileID`, for diagnostics
- byte size, if known
- creation time
- last access time, if tracked
- released/active state
- backing holder, for virtual imports

The registry is target-local process state. It does not need to be serialized as
shot data. Imported files are transient read arguments, not documented
measurement results.

## Configuration

Add a persistence-manager configuration option for imported file size limits.

Recommended key:

```text
[PersistenceManager]
importMaxBytes = 10485760
```

`10485760` bytes is 10 MiB. This should be the default when the option is not
present, since that should accommodate most expected image files while still
preventing accidental unbounded imports.

`importFile()` should check the source size before transfer when
`sourceServer->getFileSize(sourceID)` reports a positive size. If the reported
size is larger than `importMaxBytes`, the import should fail without creating a
target holder.

For sources whose size is unavailable or unreliable, the implementation should
also enforce the limit while writing the destination. If the transfer exceeds
`importMaxBytes`, the import should fail, unregister the target `FileID`, and
delete any partial temporary file or virtual buffer.

## FileID Identity and Collisions

Do not reuse the source `FileID` as the target storage identity.

The source `FileID` is a lookup key in the source server's namespace. The target
must create a new target-local `FileID` for each import by default.

Recommended collision policy:

```cpp
enum class ImportCollisionPolicy
{
    Unique,       // default: always create a fresh target FileID
    FailIfExists, // for explicit target names
    Replace       // explicit overwrite/replace
};
```

`Unique` should be the default for both virtual and disk storage. This matches
the `ResultsCollector` pattern: use the source filename as a hint, but generate
a unique destination path/name under the target's import area.

This avoids ambiguity when caller code repeatedly imports a transient file using
the same source `FileID` but different bytes.

`Replace` should be explicit. It should replace only a target import owned by
the same import registry policy, not arbitrary persistent device files.

## Source and Destination Semantics

`sourceServer` is the authority for `sourceID`.

The target persistence manager should not assume that `sourceID` belongs to the
target device's own file server, even if the ID fields look similar.

The imported target holder is a destination:

- `sourceServer->transferFile(sourceID, destination, Binary)` writes bytes into
  the target holder.
- The target file server then becomes the resolver for the returned target
  `FileID`.

For remote source servers, the destination holder must be network-exposable.
This is why import destinations must be created through the configured
`FileHolderFactory`.

## Python API Shape

Expose a small Python object:

```python
options = stipy.ImportFileOptions(
    storage=stipy.ImportStorage.DiskTemporary,
    collision=stipy.ImportCollisionPolicy.Unique,
)

with device.getPersistenceManager().importFile(source_id, source_server, options) as imported:
    result = device.read(channel, imported.fileID)
```

The object should provide:

- `fileID`
- `close()`
- `closed`
- context-manager support

The first version can require callers to provide `source_server` explicitly.
A later convenience API can infer a source virtual server from a higher-level
Python helper object.

## Future Lazy Option

A lazy import is still possible, but it should be a separate API, not the first
`importFile()` behavior.

Possible future name:

```cpp
registerExternalFile(sourceID, sourceServer, options)
```

This would create a target-local proxy `FileID` without transferring bytes. The
target file server would resolve that proxy by pulling from the stored
`sourceServer` on demand.

That design requires stronger lifetime handling because the target registry must
keep the source server reference alive until the proxy is released or expires.
It should not be mixed into the eager `importFile()` semantics.

## Implementation Steps

1. Add public import types.

   Add `ImportStorage`, `ImportCollisionPolicy`, `ImportLifetime`,
   `ImportFileOptions`, and `ImportedFile` abstractions under `include/sti`.

2. Extend `PersistenceManager`.

   Add `importFile()` and `releaseImportedFile()` or equivalent handle-backed
   release support.

3. Implement local persistence imports.

   In `LocalPersistenceManager`, create target holders through the configured
   `FileHolderFactory`, place disk-backed imports under
   `getTemporaryPath()/imports`, enforce the configured `importMaxBytes`, transfer
   bytes from `sourceServer`, register the target holder with the local file
   server when needed, and store an import-registry entry.

4. Implement remote persistence imports.

   Add IDL for import options and imported-file handles, or add a narrower RPC
   surface that imports immediately and returns a target `FileID` plus an import
   token. Keep CORBA details in `stinetwork`.

5. Add Python bindings.

   Bind the option enums and imported-file handle. Support context-manager
   syntax in Python.

6. Add tests.

   Cover local virtual import, local disk import, repeated imports with the same
   source `FileID`, explicit release cleanup, TTL cleanup behavior, and remote
   import using network-exposed destination holders.

## Test Plan

- Import a virtual source file into virtual storage, read it by target `FileID`,
  close the handle, and confirm the target server no longer resolves it.
- Import the same source `FileID` twice with different bytes and
  `collision=Unique`; confirm two distinct target `FileID`s and correct bytes.
- Import to disk temporary storage and confirm the file exists during handle
  lifetime and is removed after release.
- Verify `Replace` refuses to replace non-import-owned persistent files.
- Verify `FailIfExists` fails when the requested target name is already active.
- Verify recursive `MixedValue` vectors can contain imported `FileID`s without
  special read-path changes.
- Verify remote source transfer succeeds when the target destination holder is
  created through `NetworkFileHolderFactory`.
- Verify unreleased virtual imports are cleaned by TTL/quota only after expiry
  and not while active.
- Verify `PersistenceManager.importMaxBytes` defaults to 10 MiB and rejects
  larger imports without leaving registered or partially written target files.

## Open Questions

- Should imported files be visible in diagnostics/logs so leaked or expired
  imports are easy to inspect?
- Should `device.read()` offer a convenience wrapper that imports and releases
  automatically around a single call?
