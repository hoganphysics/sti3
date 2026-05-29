.. _deviceinterface:

Device Interface
================

The device interface is the API used after code has a reference to a device.
The reference may be local, returned by ``connect()``, obtained from a hub, or
looked up through another device's collection.  The same high-level operations
are exposed by ``LocalDevice`` and by remote device references:

* inspect connected devices with the device collection
* read and write channels
* inspect and set attributes
* subscribe to device messages
* monitor live status values
* inspect event engine and persistence results
* save and load profiles
* manage background tasks
* read logs
* inspect STI and device-specific version information

Managers
--------

Each device owns manager objects for a particular feature area.  In C++ the
manager is returned through an output ``std::shared_ptr``.  In Python the
binding returns the manager directly.

.. list-table::
   :header-rows: 1

   * - Feature
     - C++ accessor
     - Python accessor
   * - Device collection
     - ``getCollection(collection)``
     - ``getCollection()``
   * - Channels
     - ``getChannelManager(manager)``
     - ``getChannelManager()``
   * - Attributes
     - ``getAttributeManager(manager)``
     - ``getAttributeManager()``
   * - Messages
     - ``getMessageReceiver(receiver)``
     - ``getMessageReceiver()``
   * - Event engines
     - ``getEngineScheduler(scheduler)``
     - ``getEngineScheduler()``
   * - Persistence
     - ``getPersistenceManager(manager)``
     - ``getPersistenceManager()``
   * - Profiles
     - ``getProfileManager(manager)``
     - ``getProfileManager()``
   * - Monitors
     - ``getMonitorManager(manager)``
     - ``getMonitorManager()``
   * - Tasks
     - ``getTaskManager(manager)``
     - ``getTaskManager()``
   * - Logs
     - ``getLogManager(manager)``
     - ``getLogManager()``
   * - Versions
     - ``getVersionManager(manager)``
     - ``getVersionManager()``

Version information
-------------------

The ``VersionManager`` reports the STI library version used by the device and
any additional version records that the device implementation registered.  The
same API works for local devices and remote device references returned by
``connect()`` or by a device collection.

Each entry is a ``VersionInfo`` object.  The built-in STI entry uses component
name ``sti3`` and includes the package version, build number, build string, git
commit when it was available at build time, and a dirty-worktree flag.
Device-specific entries can use any component name chosen by the device
author.

.. tabs::

   .. code-tab:: c++

      #include <sti/device/VersionManager.h>

      std::shared_ptr<STI::Device::VersionManager> versions;
      if (device->getVersionManager(versions)) {
          STI::Device::VersionInfo library = versions->getLibraryVersion();
          std::cout << "STI library: " << library.toString() << std::endl;

          STI::Device::VersionInfo driver;
          if (versions->getVersion("CameraDriver", driver)) {
              std::cout << "Camera driver: " << driver.toString() << std::endl;
          }

          for (const auto& info : versions->getVersions()) {
              std::cout << info.component << " " << info.version << std::endl;
          }
      }

   .. code-tab:: py

      versions = device.getVersionManager()

      library = versions.getLibraryVersion()
      print("STI library:", library.toString())

      driver = versions.getVersion("CameraDriver")
      if driver is not None:
          print("Camera driver:", driver.toString())

      for info in versions.getVersions():
          print(info.component, info.version)

``versions.summary()`` returns a newline-separated string with all registered
version entries, which is convenient for logs and diagnostic output.

Device collection
-----------------

The device collection stores references to devices that are connected to this
device.  Use it to discover available ``DeviceID`` values and to get another
device reference.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::DeviceCollection> collection;
      device->getCollection(collection);

      std::set<STI::Device::DeviceID> ids;
      collection->getIDs(ids);

      std::shared_ptr<STI::Device::Device> other;
      STI::Device::DeviceID id("localhost/0/TestDevice");
      if (collection->get(id, other)) {
          other->write(0, 1.5);
      }

   .. code-tab:: py

      collection = device.getCollection()
      ids = collection.getIDs()

      other = collection.get("localhost/0/TestDevice")
      if other is not None:
          other.write(0, 1.5)

Channels
--------

Channels are the primary runtime I/O surface of a device.  Output channels
accept values to send to hardware.  Input channels return measurements.  An
input channel may also have an output value type, which means a read accepts an
argument.

The ``ChannelManager`` can list channel definitions, retrieve metadata, and
perform read/write operations.  The convenience methods ``device.write()``,
``device.read()``, and ``device.stopRW()`` call through to the same manager.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::ChannelManager> channels;
      device->getChannelManager(channels);

      std::vector<std::shared_ptr<STI::Device::Channel>> defs;
      channels->getChannels(defs);
      for (const auto& ch : defs) {
          std::cout << ch->getChannelNumber() << ": "
                    << ch->getChannelName() << std::endl;
      }

      channels->writeChannel(0, STI::Utils::MixedValue(2.5));

      STI::Utils::MixedValue result;
      if (channels->readChannel(10, STI::Utils::MixedValue(), result)) {
          std::cout << result.print() << std::endl;
      }

   .. code-tab:: py

      channels = device.getChannelManager()
      for ch in channels.getChannels():
          print(ch.number(), ch.name(), ch.type(), ch.metadata())

      channels.writeChannel(0, 2.5)
      result = channels.readChannel(10, None)
      print(result)

For quick access:

.. tabs::

   .. code-tab:: c++

      device->write(0, 2.5);

      STI::Utils::MixedValue data;
      device->read(10, data);

      STI::Utils::MixedValue args;
      args.addValue(12);
      args.addValue("hi");
      device->read(11, args, data);

   .. code-tab:: py

      device.write(0, 2.5)
      data = device.read(10)
      data_with_args = device.read(11, [12, "hi"])

Attributes
----------

Attributes are named string values used for configuration and operator-facing
state.  They may expose allowed values and metadata.  Setting an attribute
calls the device's setter callback if one was registered.  A successful set
also refreshes the cached string value.  Reading an attribute returns the
cached string; call refresh explicitly when local member state changes outside
the attribute setter path.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::AttributeManager> attributes;
      device->getAttributeManager(attributes);

      std::string mode = attributes->getValue("Mode");
      attributes->setValue("TriggerSource", "Software");
      attributes->refreshValue("Height");
      attributes->refreshValues();

      std::vector<std::shared_ptr<STI::Device::Attribute>> defs;
      attributes->getAttributes(defs);
      for (const auto& attr : defs) {
          std::cout << attr->getKey() << " = "
                    << attr->getValue() << std::endl;
      }

   .. code-tab:: py

      attributes = device.getAttributeManager()
      mode = attributes.getValue("Mode")
      attributes.setValue("TriggerSource", "Software")
      attributes.refreshValue("Height")
      attributes.refreshValues()

      for attr in attributes.getAttributes():
          print(attr.key(), attr.value(), attr.getAllowedValues(), attr.metadata())

The device object also has get/set convenience methods.  Local devices add
refresh convenience methods for syncing cached attributes after local member
state changes:

.. tabs::

   .. code-tab:: c++

      std::string height = localDevice->getAttribute("Height");
      bool ok = localDevice->setAttribute("Downsample", "4");
      localDevice->refreshAttribute("Height");
      localDevice->refreshAttributes();

   .. code-tab:: py

      height = device.getAttribute("Height")
      ok = device.setAttribute("Downsample", "4")
      device.refreshAttribute("Height")
      device.refreshAttributes()

Messages and listeners
----------------------

Device messages publish changes such as channel updates, attribute updates,
monitor updates, collection changes, and engine status changes.  Use the
``DeviceMessageReceiver`` when code needs to react to updates instead of
polling managers.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::DeviceMessageReceiver> receiver;
      if (localDevice->getMessageReceiver(receiver)) {
          STI::Device::DeviceID source = localDevice->getID();

          receiver->addListener<STI::Device::AttributeUpdateMessage>(
              source,
              "AttributePrinter",
              [](const std::shared_ptr<STI::Device::AttributeUpdateMessage>& mess) {
                  for (const auto& item : mess->attributes) {
                      std::cout << item.first << " = " << item.second << std::endl;
                  }
              });
      }

   .. code-tab:: py

      receiver = device.getMessageReceiver()

      def print_attribute_update(message):
          for key, value in message.attributes:
              print(key, value)

      receiver.addListener(
          stidevicepy.DeviceMessageType.AttributeUpdate,
          device.getID(),
          "AttributePrinter",
          print_attribute_update,
      )

Monitors
--------

Monitors expose live values that are not part of shot timing.  They are useful
for status, temperatures, lock states, counters, and other continuously updated
readbacks.  A monitor has an ID, status, value, and metadata.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::MonitorManager> monitors;
      if (device->getMonitorManager(monitors)) {
          std::vector<std::string> ids;
          monitors->getIDs(ids);
          for (const auto& id : ids) {
              auto value = monitors->getValue(id);
              std::cout << id << " = " << value.print() << std::endl;
          }
      }

   .. code-tab:: py

      monitors = device.getMonitorManager()
      for monitor_id in monitors.getIDs():
          print(monitor_id, monitors.getStatus(monitor_id), monitors.getValue(monitor_id))

      monitors.activateAll()

Event engine
------------

The event engine parses timing sequences into device-specific
``SynchronousEvent`` objects and plays them during a shot.  A device can host
multiple event engines so one shot can be parsed while another is ready or
playing.  Networked devices coordinate through the ``EventEngineScheduler``.

Most user code interacts with the scheduler through higher-level STIPy shot and
sequence APIs.  Device authors interact with the engine by implementing
``parseEvents()`` and by throwing parse exceptions when a sequence requests an
invalid hardware operation.

Persistence
-----------

The ``PersistenceManager`` stores parse results, shot results, sequence results,
measurements, and files produced by shots.  Use it when code needs to inspect
results after parsing or playback.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::PersistenceManager> persistence;
      if (device->getPersistenceManager(persistence)) {
          std::shared_ptr<STI::Engine::ShotResult> shot;
          if (persistence->getShotResult(shotID, shot)) {
              // inspect shot->measurements, status, and result metadata
          }
      }

   .. code-tab:: py

      persistence = device.getPersistenceManager()
      shot = persistence.getShotResult(shot_id)
      measurements = persistence.getMeasurements(shot_id)

Profiles
--------

Profiles save and restore channel and attribute state.  Use profiles for
startup states, safe states, and reproducible experiment configurations.

``ProfileType`` controls whether channels, attributes, or both are included.
The dependent-device flag also saves or loads profiles on devices for which
this device is the target server.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::ProfileManager> profiles;
      if (device->getProfileManager(profiles)) {
          profiles->saveCurrentProfile("startup", STI::Device::ProfileType::All, false);
          profiles->loadProfile("startup", STI::Device::ProfileType::All, false);

          std::set<std::string> names;
          profiles->getProfiles(names);
      }

   .. code-tab:: py

      profiles = device.getProfileManager()
      profiles.saveCurrentProfile("startup", stidevicepy.ProfileType.All, False)
      profiles.loadProfile("startup", stidevicepy.ProfileType.All, False)
      names = profiles.getProfiles()

Tasks
-----

The ``TaskManager`` exposes background tasks registered by a local device.
Tasks can be activated, deactivated, run manually, or removed.  Common task
types include interval tasks and appointment tasks.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::TaskManager> tasks;
      if (device->getTaskManager(tasks)) {
          std::set<std::string> ids;
          tasks->getTaskIDs(ids);
          tasks->deactivateTask("task#1");
          tasks->runTask("task#2");
      }

   .. code-tab:: py

      tasks = device.getTaskManager()
      print(tasks.getTaskIDs())
      tasks.deactivateTask("task#1")
      tasks.runTask("task#2")

Logs
----

The log manager lists logs generated by a device and can retrieve local or
network log files.  Device implementations write logs through ``log()``; client
code reads them through ``LogManager``.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::LogManager> logs;
      if (device->getLogManager(logs)) {
          std::set<std::string> names;
          logs->getLogNames(names);
      }

   .. code-tab:: py

      logs = device.getLogManager()
      names = logs.getLogNames()
