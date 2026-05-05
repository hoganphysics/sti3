# LogBrowser FileHolder Refactor Plan

## Goal

Refactor `LogBrowser` so `stidevicepy` does not include `stinetwork` headers or require omniORB compile definitions. `stinetwork` should remain the layer that knows how to expose a local `FileHolder` over CORBA.

## Current Problem

`src/stipy/stidevicepy/src/LogBrowser.cpp` includes:

- `NetworkFileHolder.h`
- `RemoteFileServer.h`

This makes `sticommonpy` compile through generated CORBA headers. On Windows, that exposes omniORB platform configuration requirements to `stidevicepy`, which breaks the intended library boundary.

The direct dependency exists because `LogBrowser` creates a `NetworkFileHolder` when it needs to receive log text from a remote `FileServer`.

## Intended Design

`LogBrowser` should treat file transfer destinations abstractly:

- It should ask a `PersistenceManager` or `FileHolderFactory` for destination `FileHolder` instances.
- It should pass those holders to `FileServer::transferFilePartial()` or `FileServer::transferFile()`.
- It should not know whether the destination holder is local-only or network-exposed.

The concrete behavior should be:

- `LocalPersistenceManager::makeVirtualFileHolder()` returns a local in-memory `VirtualFileHolder`.
- `RemotePersistenceManager::makeVirtualFileHolder()` returns a client-local `VirtualFileHolder` wrapped in `NetworkFileHolder`, exposed as `std::shared_ptr<FileHolder>`.
- `RemoteFileServer` remains responsible for converting the abstract `FileHolder` destination into a CORBA `TFileHolder` reference.

## Important Source/Destination Semantics

`FileHolder` can act in two ways:

- Source: `FileHolder::transferFile(destination)` reads from itself and writes to another holder.
- Destination: `openFile()`, `write()`, and `closeFile()` receive bytes.

For `LogBrowser`, the holder made by the factory is a destination. It receives log text from the source device's `FileServer`.

This makes it logically valid for `RemotePersistenceManager` to create destination holders. It is not claiming to create a reference to a file on the remote filesystem; it is creating a local receiver that can be passed to the remote host through CORBA.

## Implementation Plan

1. Revert the CMake workaround from the previous direction.

   Remove:

   - `cmake/StiOmniORB.cmake`
   - omniORB helper includes in `src/network/src/CMakeLists.txt`
   - omniORB compile definitions added to `sticommonpy`, `stipybase`, `stidevicepy`, and `stipy`
   - wrapper link-line changes that were only needed because `LogBrowser` included omniORB headers

   Keep the conda IDL-generation changes unless we decide to revise them separately.

2. Change `LogBrowser` ownership state.

   Update `LogBrowser` to store:

   - `std::shared_ptr<STI::Device::LogManager> sourceLogManager`
   - `std::shared_ptr<STI::Utils::FileServer> sourceFileServer`
   - `std::shared_ptr<STI::Utils::FileHolderFactory> destinationFileHolderFactory`

   The destination factory is the source device's persistence manager, viewed through the `FileHolderFactory` interface. This is the manager that knows whether the destination holder must be network-exposed for transfers from the source file server.

3. Remove network headers from `LogBrowser.cpp`.

   Remove:

   - `#include "NetworkFileHolder.h"`
   - `#include "RemoteFileServer.h"`
   - `using STI::Network::NetworkFileHolder`
   - `using STI::Network::RemoteFileServer`
   - `CORBA::Exception` catch blocks

   Keep only device and utils abstractions.

4. Create virtual transfer destinations through the factory.

   Add an overload to `FileHolderFactory` that lets callers provide the readable virtual buffer:

   ```cpp
   virtual std::shared_ptr<FileHolder> makeVirtualFileHolder(
       const std::shared_ptr<VirtualFileHolder>& backingHolder) = 0;
   ```

   This overload does not create a new virtual file. It wraps or returns a caller-owned `VirtualFileHolder` so the caller can still read from it after transfer.

   Replace direct construction of `NetworkFileHolder` in `transferText()` with:

   - create a local readable `VirtualFileHolder` in `LogBrowser`
   - ask `destinationFactory->makeVirtualFileHolder(readableVirtualHolder)` for the transfer destination
   - call `sourceFileServer->transferFilePartial(..., destination, ...)`

   `LogBrowser` keeps the original `readableVirtualHolder`, so it can read the transferred text without needing to know whether `destination` is the same holder or a `NetworkFileHolder` wrapper.

   This keeps `LogBrowser` independent of `NetworkFileHolder` while still allowing remote transfer callbacks.

   A more general `wrapFileHolder(const std::shared_ptr<FileHolder>& backingHolder)` could also work, but start with the narrower virtual-holder overload unless another caller needs general wrapping.

5. Create save destinations through the factory.

   In `saveLocal()`, replace direct `LocalFileHolder` / `NetworkFileHolder` creation with:

   - derive the destination directory and filename from the requested path
   - call `destinationFactory->makeFileHolder(directory, filename)`
   - pass the returned `FileHolder` to `sourceFileServer->transferFile(...)`

   For a remote persistence manager, this destination should be a local file holder wrapped in `NetworkFileHolder`.

6. Pass the factory into `LogBrowser`.

   In `openLog()`:

   - resolve the source device from the log ID
   - get the source log manager from the source device
   - get the source persistence manager from the source device
   - get the source file server from that persistence manager
   - pass that same persistence manager to `LogBrowser` as the destination `FileHolderFactory`
   - construct `LogBrowser(sourceLogManager, sourceFileServer, sourcePersistenceManager, logFileRecord)`

   This means local source devices create plain local holders, while remote source devices create local holders wrapped in `NetworkFileHolder` for the CORBA callback.

7. Implement missing `RemotePersistenceManager` factory methods.

   In `src/network/src/RemotePersistenceManager.cpp`:

   - `makeVirtualFileHolder(fileID)` should create a local `VirtualFileHolder`, wrap it in `NetworkFileHolder`, and return it as `std::shared_ptr<FileHolder>`.
   - `makeVirtualFileHolder(backingHolder)` should wrap the provided local `VirtualFileHolder` in `NetworkFileHolder` and return it as `std::shared_ptr<FileHolder>`.
   - `makeFileHolder(path, filename)` should create a local `LocalFileHolder`, wrap it in `NetworkFileHolder`, and return it as `std::shared_ptr<FileHolder>`.

   The path in `makeFileHolder()` is interpreted on the local client machine, not the remote device host.

8. Implement the new virtual-holder overload in all factories.

   Update each `FileHolderFactory` implementation:

   - `LocalFileHolderFactory::makeVirtualFileHolder(backingHolder)` returns `backingHolder` as `std::shared_ptr<FileHolder>`.
   - `NetworkFileHolderFactory::makeVirtualFileHolder(backingHolder)` wraps `backingHolder` in `NetworkFileHolder`.
   - `LocalPersistenceManager::makeVirtualFileHolder(backingHolder)` delegates to its configured `fileHolderFactory`.
   - `RemotePersistenceManager::makeVirtualFileHolder(backingHolder)` wraps `backingHolder` in `NetworkFileHolder`.

9. Consider naming/documentation follow-up.

   The existing factory names are ambiguous because they do not say whether the holder is intended as a source or destination. Add comments near `RemotePersistenceManager::makeFileHolder()` and `makeVirtualFileHolder()` explaining that these create local destination holders for remote transfers.

## Files To Change

- `src/stipy/stidevicepy/src/LogBrowser.h`
- `src/stipy/stidevicepy/src/LogBrowser.cpp`
- `include/sti/utils/FileHolderFactory.h`
- `include/sti/utils/LocalFileHolder.h`
- `src/device/src/LocalFileHolder.cpp`
- `src/device/src/LocalPersistenceManager.h`
- `src/device/src/LocalPersistenceManager.cpp`
- `src/network/src/NetworkFileHolder.h`
- `src/network/src/NetworkFileHolder.cpp`
- `src/network/src/RemotePersistenceManager.cpp`
- Possibly `src/network/src/RemotePersistenceManager.h` if comments or const correctness are adjusted
- Revert recent CMake changes in:
  - `cmake/StiOmniORB.cmake`
  - `src/network/src/CMakeLists.txt`
  - `src/stipy/stidevicepy/src/CMakeLists.txt`
  - `src/stipy/src/CMakeLists.txt`

## Verification

1. Configure:

   ```sh
   cd build-ninja && conda run --no-capture-output -n sti3-build cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PYTHONDIR=Lib/site-packages ..
   ```

2. Build:

   ```sh
   cd build-ninja && conda run --no-capture-output -n sti3-build cmake --build . --parallel 8
   ```

3. Run tests:

   ```sh
   ctest --test-dir build-ninja --output-on-failure
   ```

4. Confirm compile boundaries:

   - `LogBrowser.cpp` compile command should not include `src/network/src/generated`.
   - `LogBrowser.cpp` should not include omniORB headers.
   - `sticommonpy` should not need omniORB compile definitions.

## Risks

- Adding a pure virtual overload to `FileHolderFactory` requires updating every factory implementation. Current known implementations are small, so this is manageable.
- `LogBrowser` must keep the caller-owned `VirtualFileHolder` alive for at least the duration of the transfer and read-back.
- `RemotePersistenceManager::makeFileHolder()` must be clearly documented as creating a local destination file, not a remote file reference.
- Any existing callers that assumed `RemotePersistenceManager::makeFileHolder()` returns null should be checked, though current searches suggest these methods are effectively unimplemented today.
