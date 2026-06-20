.. _devicelib:

==============
Device library
==============

For controlling an existing device reference, see :ref:`deviceinterface`.


Creating a device
-----------------

An STI device driver is a standalone program that owns one or more
``LocalDevice`` instances and attaches them to a ``NetworkDeviceHub``.  The
driver defines channels, attributes, monitors, parsing behavior, tasks, logs,
and any partner-device relationships needed by the hardware.

The STI3 examples are organized by feature under ``examples/cpp`` and
``examples/python``.  The snippets below follow the current C++ and Python
interfaces used by those examples.

Minimal device
**************

Every device derives from ``LocalDevice``.  The constructor can take explicit
ID fields or a ``Configuration`` object.  The ID fields are the device name, IP
address string, module number, and target server ``DeviceID``.

.. tabs::

   .. code-tab:: c++

      #include <sti/LocalDevice.h>

      class SimpleDevice : public STI::Device::LocalDevice
      {
      public:
          SimpleDevice(const STI::Utils::Configuration& config)
              : STI::Device::LocalDevice(config)
          {
          }
      };

   .. code-tab:: py

      import stipy
      import stipy.stidevicepy as stidevicepy

      class SimpleDevice(stidevicepy.LocalDevice):
          def __init__(self, config):
              stidevicepy.LocalDevice.__init__(self, config)

Run the device by adding it to a hub:

.. tabs::

   .. code-tab:: c++

      #include <sti/NetworkDeviceHub.h>

      int main()
      {
          STI::Utils::Configuration config({
              {"Device Name", "SimpleDevice"},
              {"IP Address", "localhost"},
              {"Module", "0"},
              {"Target Server", "localhost/0/STI Server"},
          });

          auto device = std::make_shared<SimpleDevice>(config);
          auto hub = std::make_shared<STI::Network::NetworkDeviceHub>("192.168.1.4:2809");

          hub->addDevice(device);
          hub->run();
      }

   .. code-tab:: py

      config = stipy.Configuration({
          "Device Name": "SimpleDevice",
          "IP Address": "localhost",
          "Module": "0",
          "Target Server": "localhost/0/STI Server",
      })

      device = SimpleDevice(config)
      hub = stidevicepy.NetworkDeviceHub("192.168.1.4:2809")
      hub.addDevice(device)
      hub.run()

``NetworkDeviceHub`` needs the omniORB name service address.  ``run()`` blocks
by default; pass ``false`` in C++ or ``False`` in Python to return immediately
after starting the hub.

Configuration files
*******************

Examples in ``examples/cpp/configFile`` and ``examples/python/configFile`` show
how to load device and network settings from an INI-style file.  This keeps
deployment-specific values out of the driver source.

.. tabs::

   .. code-tab:: c++

      STI::Utils::ConfigFile configFile("testDevice.ini");

      auto device = std::make_shared<TestDevice>(configFile);
      auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(configFile);

   .. code-tab:: py

      config = stipy.ConfigFile("testDevice.ini")

      device = TestDevice(config)
      hub = stidevicepy.NetworkDeviceHub(config)

Device-specific version information
***********************************

Every ``LocalDevice`` automatically reports the STI library version it is using.
Device authors can add an additional record for the device driver, hardware
support package, or firmware using ``addVersionInfo``.  Add this during device
construction so clients can query it immediately after connecting.

The short form is enough when the device only needs a component name and a
semantic version string.  The version string is parsed into ``major``,
``minor``, and ``patch`` fields when it has a ``major.minor.patch`` shape.

.. tabs::

   .. code-tab:: c++

      #include <sti/device/VersionInfo.h>

      class SimpleDevice : public STI::Device::LocalDevice
      {
      public:
          SimpleDevice(const STI::Utils::Configuration& config)
              : STI::Device::LocalDevice(config)
          {
              addVersionInfo("SimpleDevice", "1.4.0");
          }
      };

   .. code-tab:: py

      class SimpleDevice(stidevicepy.LocalDevice):
          def __init__(self, config):
              stidevicepy.LocalDevice.__init__(self, config)
              self.addVersionInfo("SimpleDevice", "1.4.0")

Use ``VersionInfo`` directly when the device has richer build metadata.  Entries
with the same component name replace the previous entry, which lets a device
refresh its own record without duplicating it.

.. tabs::

   .. code-tab:: c++

      STI::Device::VersionInfo driver("CameraDriver", "2.1.3");
      driver.buildNumber = 42;
      driver.gitCommit = "8f12c0a";
      driver.gitDirty = false;
      driver.metadata["firmware"] = "5.7.0";
      driver.metadata["board"] = "rev-c";

      addVersionInfo(driver);

   .. code-tab:: py

      driver = stidevicepy.VersionInfo("CameraDriver", "2.1.3")
      driver.buildNumber = 42
      driver.gitCommit = "8f12c0a"
      driver.gitDirty = False
      driver.metadata["firmware"] = "5.7.0"
      driver.metadata["board"] = "rev-c"

      self.addVersionInfo(driver)

Defining channels
*****************

Channels describe what the timing system and clients can read or write.
``MixedValueType`` declares the value type accepted by the channel.

* Output channels receive values through ``write()`` or timing events.
* Input channels produce measurements through ``read()`` or measurement events.
* Input channels can also accept an output argument for parameterized reads.

.. tabs::

   .. code-tab:: c++

      using STI::Utils::MixedValueType;

      addOutputChannel(0, MixedValueType::Double, "coil current");
      addOutputChannel(1, MixedValueType::Int, "temperature setpoint");
      addOutputChannel(3, MixedValueType::Vector, "list output")
          .setVectorFormat({MixedValueType::Number,
                            MixedValueType::String,
                            MixedValueType::Boolean})
          .setUnits("arb")
          .setHelp("Vector format is [frequency, label, enable].");

      addInputChannel(10, MixedValueType::Number, "thermocouple voltage");
      addInputChannel(11, MixedValueType::Number, MixedValueType::Vector, "vector args");

   .. code-tab:: py

      ch = self.addOutputChannel(0, stipy.MixedValueType.Double, "coil current")
      ch.setUnits("A").setMinValue(stipy.MixedValue(-10.0)).setMaxValue(stipy.MixedValue(10.0))

      self.addOutputChannel(1, stipy.MixedValueType.Int, "temperature setpoint")

      ch = self.addOutputChannel(3, stipy.MixedValueType.Vector, "list output")
      ch.setVectorFormat([
          stipy.MixedValueType.Number,
          stipy.MixedValueType.String,
          stipy.MixedValueType.Boolean,
      ]).setValueHint("[Frequency (MHz), name, enable]")

      self.addInputChannel(10, stipy.MixedValueType.Number, "thermocouple voltage")
      self.addInputChannel(11, stipy.MixedValueType.Number, stipy.MixedValueType.Vector, "vector args")

Implementing read and write
***************************

Override ``writeChannel`` and ``readChannel`` for software-controlled channel
I/O.  The public ``write`` and ``read`` methods validate against the channel
definition and then call these hooks.

.. tabs::

   .. code-tab:: c++

      bool TestDevice::writeChannel(short channel, const STI::Utils::MixedValue& value)
      {
          switch (channel) {
          case 0:
              hardware.setCoilCurrent(value.getDouble());
              return true;
          case 3:
              if (!value.isType({STI::Utils::MixedValueType::Number,
                                 STI::Utils::MixedValueType::String,
                                 STI::Utils::MixedValueType::Boolean})) {
                  return false;
              }
              return hardware.sendVectorCommand(value.getVector());
          default:
              return false;
          }
      }

      bool TestDevice::readChannel(short channel,
                                   const STI::Utils::MixedValue& value,
                                   STI::Utils::MixedValue& data)
      {
          if (channel == 10) {
              data.setValue(hardware.readTemperature());
              return true;
          }
          if (channel == 11 && value.isType({STI::Utils::MixedValueType::Number,
                                             STI::Utils::MixedValueType::String})) {
              data.setValue(hardware.readWithArgs(value.getVector()));
              return true;
          }
          return false;
      }

   .. code-tab:: py

      def writeChannel(self, channel, value):
          if channel == 0:
              self.hardware.set_coil_current(value)
              return True
          if channel == 3:
              mval = stipy.MixedValue()
              mval.setValue(value)
              if not mval.isType([
                  stipy.MixedValueType.Number,
                  stipy.MixedValueType.String,
                  stipy.MixedValueType.Boolean,
              ]):
                  return False
              self.hardware.send_vector_command(value)
              return True
          return False

      def readChannel(self, channel, value):
          if channel == 10:
              return self.hardware.read_temperature()
          if channel == 11:
              return self.hardware.read_with_args(value)
          return None

FileID arguments for reads and writes
*************************************

``MixedValueType::File`` values carry a ``FileID`` only.  They do not carry the
``FileServer`` that can serve the bytes.  When a caller wants to pass a file to
``device.write()`` or to a parameterized ``device.read()``, import the source
file into the target device's ``PersistenceManager`` first, then pass the
returned target-side ``FileID`` as the channel value.

``PersistenceManager::importFile()`` eagerly copies the file from a source
``FileServer`` into target persistence storage.  ``DiskTemporary`` is the
default storage mode and writes under the target persistence manager's temporary
directory.  ``Virtual`` can be requested when the target should hold the import
in its virtual file server instead.  The returned ``ImportedFile`` handle owns
the imported registration; close it, or use the Python context manager, after
the read or write is complete.

.. tabs::

   .. code-tab:: c++

      #include <sti/device/ImportedFile.h>
      #include <sti/device/PersistenceManager.h>
      #include <sti/utils/FileHolder.h>
      #include <sti/utils/FileServer.h>
      #include <sti/utils/MixedValue.h>

      std::shared_ptr<STI::Device::PersistenceManager> targetPersistence;
      targetDevice->getPersistenceManager(targetPersistence);

      std::shared_ptr<STI::Utils::FileServer> sourceServer;
      sourcePersistence->getFileServer(sourceServer);

      STI::Device::ImportFileOptions options;  // DiskTemporary, Unique, Handle
      auto imported = targetPersistence->importFile(sourceHolder->getID(),
                                                   sourceServer,
                                                   options);
      if (imported != nullptr) {
          STI::Utils::MixedValue fileArg(imported->getFileID());

          targetDevice->write(6, fileArg);

          STI::Utils::MixedValue result;
          targetDevice->read(20, fileArg, result);

          imported->close();
      }

   .. code-tab:: py

      target_persistence = target_device.getPersistenceManager()
      source_server = source_persistence.getFileServer()

      with target_persistence.importFile(source_holder.getID(), source_server) as imported:
          target_device.write(6, imported.fileID)
          size = target_device.read(20, imported.fileID)

For caller-created files in Python, create an explicit source server and
register the holder before importing.  The source server is what lets the
target pull the bytes; the ``FileID`` alone is not enough.

.. code-block:: py

   import tempfile

   source_server = target_persistence.makeVirtualFileServer()
   source_holder = target_persistence.makeFileHolder(tempfile.gettempdir(), "payload.bin")

   assert source_holder.openFile()
   try:
       source_holder.writeBytes(payload)
   finally:
       source_holder.closeFile()

   source_server.addFile(source_holder)

   with target_persistence.importFile(source_holder.getID(), source_server) as imported:
       target_device.write(6, imported.fileID)

Use ``ImportFileOptions`` when the default temporary disk import is not the
right target storage:

.. code-block:: py

   options = stipy.ImportFileOptions(storage=stipy.ImportStorage.Virtual)

   with target_persistence.importFile(source_id, source_server, options) as imported:
       target_device.read(20, imported.fileID)

Repeated imports default to ``ImportCollisionPolicy.Unique`` so transient files
with the same source name do not overwrite each other.  ``FailIfExists`` and
``Replace`` are available for callers that need stricter name handling.

The import size limit is configured on the target persistence manager.  The
default is 10 MB.

.. code-block:: ini

   [PersistenceManager]
   importMaxBytes = 10485760

This import path is for file arguments passed into ``read()`` and ``write()``.
Files returned by reads, measurements, channel updates, or shot results use the
normal result-transfer and lazy-payload paths described below.

Device attributes
*****************

Attributes are string-valued configuration fields.  Use setter callbacks to
validate and apply a new value to hardware or member state.  Use refresher
callbacks to synchronize the displayed value with current state.

.. tabs::

   .. code-tab:: c++

      addAttribute("Downsample", 1)
          .setSetter([this](const std::string& value) {
              int ds = 0;
              if (STI::Utils::stringToValue(value, ds) && ds > 0) {
                  downsample = ds;
                  return true;
              }
              return false;
          })
          .setRefresher([this]() {
              return STI::Utils::valueToString(downsample);
          });

      addAttribute("TriggerSource", "Hardware", {"Hardware", "Software"})
          .setSetter([this](const std::string& value) {
              hardwareTrigger = (value == "Hardware");
              return true;
          })
          .addMetaData("help", "Selects the trigger source.");

   .. code-tab:: py

      self.addAttribute("Downsample", "1") \
          .setSetter(self.set_downsample) \
          .setRefresher(lambda: str(self.downsample))

      self.addAttribute("TriggerSource", "Hardware", ["Hardware", "Software"]) \
          .setSetter(self.set_trigger_source) \
          .addMetadata("help", "Selects the trigger source.")

      def set_downsample(self, value):
          ds = int(value)
          if ds <= 0:
              return False
          self.downsample = ds
          return True

      def set_trigger_source(self, value):
          self.hardware_trigger = (value == "Hardware")
          return True

Parsing timing events
*********************

Timing files produce ``RawEvent`` objects grouped by event time.  A device
turns those raw events into hardware-specific ``SynchronousEvent`` objects by
overriding ``parseEvents``.

During parsing:

* validate requested channels, values, and hardware constraints
* throw ``EventParsingException`` for invalid single events
* throw ``EventConflictException`` when two events cannot coexist
* call ``addMeasurement(rawEvent)`` on any synchronous event that will produce
  measurement data
* append each generated event to ``synchedEvents``

.. tabs::

   .. code-tab:: c++

      void TestDevice::parseEvents(const STI::Engine::RawEventMap& eventsIn,
                                   STI::Engine::SynchronousEventVector& synchedEvents)
      {
          for (const auto& [time, events] : eventsIn) {
              bool hasInput = false;
              for (const auto& event : events) {
                  hasInput = hasInput || event.isMeasurementEvent();
              }

              if (hasInput && events.size() > 1) {
                  throw STI::Engine::EventConflictException(
                      events.at(0), events.at(1),
                      "Input events must be scheduled by themselves.");
              }

              if (hasInput) {
                  auto input = std::make_shared<InputEvent>(time);
                  input->addMeasurement(events.front());
                  synchedEvents.push_back(input);
              }
              else {
                  auto output = std::make_shared<OutputEvent>(time);
                  for (const auto& event : events) {
                      if (event.value().getNumber() > 10) {
                          throw STI::Engine::EventParsingException(
                              event, "Requested value exceeds hardware limit.");
                      }
                      output->addValue(event.channel(), event.value());
                  }
                  synchedEvents.push_back(output);
              }
          }
      }

   .. code-tab:: py

      def parseEvents(self, eventsIn, synchedEvents):
          for time, events in eventsIn.items():
              has_input = any(evt.isMeasurementEvent() for evt in events)

              if has_input and len(events) > 1:
                  raise stipy.EventConflictException(
                      events[0], events[1],
                      "Input events must be scheduled by themselves.",
                  )

              if has_input:
                  event = InputEvent(time)
                  event.addMeasurement(events[0])
                  synchedEvents.append(event)
              else:
                  event = OutputEvent(time)
                  for raw in events:
                      if raw.value().getValue() > 10:
                          raise stipy.EventParsingException(
                              raw, "Requested value exceeds hardware limit."
                          )
                      event.addValue(raw.channel(), raw.value())
                  synchedEvents.append(event)

Synchronous events
******************

``SynchronousEvent`` is the hardware-level event object played by the engine.
Implement the hook methods that matter for the hardware.  ``loadEvent`` runs
before playback, ``playEvent`` runs at the scheduled time, and
``collectMeasurementData`` runs after playback to attach measurements.

.. tabs::

   .. code-tab:: c++

      class OutputEvent : public STI::Engine::SynchronousEventAdapter
      {
      public:
          explicit OutputEvent(double time)
              : STI::Engine::SynchronousEventAdapter(time)
          {
          }

          void addValue(short channel, const STI::Utils::MixedValue& value)
          {
              values[channel] = value;
          }

          void loadEvent() override
          {
              for (const auto& [channel, value] : values) {
                  hardwareLoad(channel, value);
              }
          }

          void playEvent() override
          {
              hardwareTrigger();
          }

      private:
          std::map<short, STI::Utils::MixedValue> values;
      };

   .. code-tab:: py

      class OutputEvent(stidevicepy.SynchronousEvent):
          def __init__(self, time):
              stidevicepy.SynchronousEvent.__init__(self, time)
              self.values = {}

          def addValue(self, channel, value):
              self.values[channel] = value

          def loadEvent(self):
              for channel, value in self.values.items():
                  hardware_load(channel, value)

          def playEvent(self):
              hardware_trigger()

          def collectMeasurementData(self):
              return

          def stopEvent(self):
              hardware_stop()

          def pauseEvent(self):
              return

          def unpauseEvent(self, retrigger):
              return

For measurement events, set the measurement result during collection:

.. tabs::

   .. code-tab:: c++

      void InputEvent::collectMeasurementData()
      {
          STI::Utils::MixedValue measured;
          measured.setValue(hardwareRead());
          setMeasurementResult(measured);
      }

   .. code-tab:: py

      def collectMeasurementData(self):
          for measurement in self.getMeasurements():
              measurement.setMeasurementResult(hardware_read())

Partner devices and partner events
**********************************

A device can declare partner devices by ``DeviceID``.  A partner can be used
for normal channel and attribute I/O.  If the local device also declares the
partner as an event target, ``parseEvents`` can add timing events for that
partner while parsing local events.

.. tabs::

   .. code-tab:: c++

      STI::Device::DeviceID supplyID("localhost/0/Supply");
      addPartner(supplyID, "supply");
      addEventTarget(supplyID, "supply");

      partner("supply").write(0, 1.2);
      partner("supply").setAttribute("Mode", "Remote");

   .. code-tab:: py

      supply_id = stipy.DeviceID("localhost/0/Supply")
      self.addPartner(supply_id, "supply")
      self.addEventTarget(supply_id, "supply")

      self.partner("supply").write(0, 1.2)
      self.partner("supply").setAttribute("Mode", "Remote")

Device monitors
***************

Use monitors for live status values.  ``addMonitor`` creates a manually updated
monitor.  ``addAutoMonitor`` creates a monitor whose callback runs on an
interval.

.. tabs::

   .. code-tab:: c++

      auto& state = addMonitor("Status/state")
          .addMetaData("help", "Current device state.");
      state.setValue("Idle");

      addAutoMonitor("Status/temperatureC", 1.0, [this]() {
          return STI::Utils::MixedValue(readTemperatureC());
      }).addMetaData("units", "C");

   .. code-tab:: py

      self.stateMonitor = self.addMonitor("Status/state") \
          .addMetadata("help", "Current device state.") \
          .setValue("Idle")

      self.temperatureMonitor = self.addAutoMonitor(
          "Status/temperatureC",
          1.0,
          self.read_temperature_c,
      ).addMetadata("units", "C")

      def read_temperature_c(self):
          return 22.0

Device tasks
************

Tasks are background work owned by the device.  Use ``IntervalTask`` for fixed
period work, ``AppointmentTask`` for a time-of-day task, or derive from
``Task`` for custom scheduling.

.. tabs::

   .. code-tab:: c++

      auto interval = std::make_shared<STI::Utils::IntervalTask>(
          "field poll",
          "00:00:02",
          [this]() {
              STI::Utils::MixedValue data;
              read(11, data);
              log("tasks") << "field = " << data.print() << std::endl;
          });
      addTask(interval);

      auto appointment = std::make_shared<STI::Utils::AppointmentTask>(
          "daily reset",
          "08:00:00",
          STI::Utils::AppointmentTask::AppointmentRepeatType::Everyday,
          [this]() { write(0, 0.0); });
      addTask(appointment);

   .. code-tab:: py

      def poll_field():
          self.log("tasks").append(f"field = {self.read(11)}")

      self.addTask(stipy.IntervalTask("field poll", "00:00:02", poll_field))

      self.addTask(stipy.AppointmentTask(
          "daily reset",
          "08:00:00",
          stipy.AppointmentRepeatType.Everyday,
          lambda: self.write(0, 0.0),
      ))

Logging
*******

Use ``log()`` for the default log and ``log(name)`` for a named log.  Logs can
also schedule recurring read, write, and attribute log tasks.

.. tabs::

   .. code-tab:: c++

      log() << "Constructing TestDevice" << std::endl;
      log("testing") << "A log comment" << std::endl;

      log().addAttributeLogTask("x", "00:00:05");
      log().addWriteLogTask(0, "00:00:06", STI::Utils::MixedValue(11.2));

      STI::Utils::MixedValue args;
      args.addValue(5.7);
      args.addValue("example data");
      log("testing").addReadLogTask(11, "00:00:02", args);

   .. code-tab:: py

      self.log().append("Constructing TestDevice")
      self.log("testing").append("A log comment")

      self.log().addAttributeLogTask("x", "00:00:05")
      self.log().addWriteLogTask(0, "00:00:06", 11.2)
      self.log("testing").addReadLogTask(11, "00:00:02", [5.7, "example data"])

Channel cached state
********************

Channels keep two lifetime-scoped cached values.  ``lastValue`` is the
output-side value: the most recent output write, or for input channels with a
non-empty output type, the most recent read argument/configuration value.
Input channels with ``MixedValueType::Empty`` output type intentionally keep
``lastValue`` empty.  ``lastMeasurement`` is the most recent data returned by
an input channel measurement.  Output channels keep ``lastMeasurement`` empty.
Remote ``lastMeasurement`` values and remote read results that contain binary
or image data may be delivered as lazy stream-backed values.  See
:ref:`lazy_payloads` for the C++ and Python APIs used to inspect, pull, and
save those payloads.

Device profiles
***************

Profiles save and restore the current channel and attribute state.  Device
authors usually only need to define the channels and attributes correctly; the
profile manager handles saving and loading.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::ProfileManager> profiles;
      if (getProfileManager(profiles)) {
          profiles->saveCurrentProfile("safe", STI::Device::ProfileType::All, false);
          profiles->loadProfile("safe", STI::Device::ProfileType::All, false);
      }

   .. code-tab:: py

      profiles = self.getProfileManager()
      profiles.saveCurrentProfile("safe", stidevicepy.ProfileType.All, False)
      profiles.loadProfile("safe", stidevicepy.ProfileType.All, False)

Message listeners
*****************

Local devices can listen to their own messages or to messages from partner
devices.  The C++ API uses typed listeners.  The Python helper dispatches based
on ``DeviceMessageType``.

.. tabs::

   .. code-tab:: c++

      std::shared_ptr<STI::Device::DeviceMessageReceiver> receiver;
      if (getMessageReceiver(receiver)) {
          receiver->addListener<STI::Device::ChannelUpdateMessage>(
              getID(),
              "channel listener",
              [](const std::shared_ptr<STI::Device::ChannelUpdateMessage>& mess) {
                  for (const auto& update : mess->channelValues) {
                      std::cout << "value " << update.first << " -> "
                                << update.second.print() << std::endl;
                  }
                  for (const auto& update : mess->measurementValues) {
                      std::cout << "measurement " << update.first << " -> "
                                << update.second.print() << std::endl;
                  }
              });
      }

   .. code-tab:: py

      receiver = self.getMessageReceiver()

      def on_channel_update(message):
          for channel, value in message.channelValues().items():
              print("value", channel, value)
          for channel, value in message.measurementValues().items():
              print("measurement", channel, value)

      receiver.addListener(
          stidevicepy.DeviceMessageType.ChannelUpdate,
          self.getID(),
          "channel listener",
          on_channel_update,
      )

File measurements
*****************

Measurement events can attach scalar data through ``setMeasurementResult``.
For file-producing hardware, use the device persistence/file APIs to make a
file holder and attach file-backed data to the result.  See
``examples/cpp/fileMeasurement`` for the current C++ pattern.

.. _lazy_payloads:

Lazy binary and image payloads
******************************

Binary and image measurements can be large enough that copying them into every
remote read response or channel update is wasteful.  For remote clients, STI
keeps the public API value intact but may defer the actual bytes until the
client asks for them.

This applies to heavy values delivered through remote read calls, remote
channel snapshots, and channel update messages:

* ``MixedValueType::Binary`` / ``stipy.MixedValueType.Binary``
* ``MixedValueType::Image`` / ``stipy.MixedValueType.Image`` when the image
  contains inline binary image data

Remote clients receive a lightweight ``BinaryData`` or ``Image`` object for
these values.  Metadata such as total byte count, element length, word size,
image width, image height, and file ID is available before the bytes are
pulled.

Normal device-driver code should use the public utility types from
``include/sti``.  Transport details are internal to the network layer; C++
device, STIPy, and utility code do not need network-specific headers to publish
or consume these payloads.

When data is pulled
+++++++++++++++++++

A lazy ``BinaryData`` has stream metadata but no local byte buffer.  The bytes
are transferred when code explicitly materializes it, saves it, or asks for the
byte buffer through an API that requires local data.

Use the metadata methods first when possible:

* ``length()`` is the element count.
* ``wordsize()`` is the byte size of each element.
* ``bytes()`` is ``length() * wordsize()``.
* ``hasStream()`` means the object can pull from a deferred source.
* ``hasLocalData()`` / ``isMaterialized()`` means the bytes are already local.

Stream-backed payloads are transferred as byte chunks.  The original
``wordsize()`` is preserved so callers can interpret typed payloads, but the
materialized buffer should be treated as byte-addressable data.

Lazy references depend on the source data still being available.  Pull the data
before the producing device exits, disconnects, or replaces the channel's last
measurement if the client needs a durable local copy.

C++ client access
+++++++++++++++++

Use ``MixedValue::getBinary()`` and ``MixedValue::getImage()`` on remote read
results or cached measurements when the caller wants explicit control over the
transfer.

.. code-block:: c++

   #include <sti/device/Channel.h>
   #include <sti/utils/BinaryData.h>
   #include <sti/utils/FileHolder.h>
   #include <sti/utils/Image.h>
   #include <sti/utils/MixedValue.h>

   using STI::Utils::MixedValueType;

   STI::Utils::MixedValue measurement;
   if (!device->read(0, measurement)) {
       return;
   }

   // The same pattern applies to channel->getLastMeasurement().

   if (measurement.getType() == MixedValueType::Binary) {
       auto binary = measurement.getBinary();

       if (binary != nullptr) {
           auto totalBytes = binary->bytes();
           auto wordSize = binary->wordsize();

           if (binary->hasStream() && !binary->isMaterialized()) {
               if (!binary->materialize()) {
                   // The source was not available or the transfer failed.
                   return;
               }
           }

           char* bytes = nullptr;
           if (binary->getBytes(bytes)) {
               processBytes(bytes, totalBytes, wordSize);
           }
       }
   }

For image measurements, inspect the image metadata first.  Inline image data is
available through ``Image::getData()`` as ``BinaryData`` and follows the same
lazy materialization rules.

.. code-block:: c++

   auto measurement = channel->getLastMeasurement();

   if (measurement.getType() == MixedValueType::Image) {
       auto image = measurement.getImage();

       if (image != nullptr) {
           auto width = image->getWidth();
           auto height = image->getHeight();
           auto fileID = image->getFileID();

           std::shared_ptr<STI::Utils::BinaryData> imageData;
           if (image->getData(imageData) && imageData != nullptr) {
               if (imageData->materialize()) {
                   char* bytes = nullptr;
                   if (imageData->getBytes(bytes)) {
                       processImageBytes(bytes, imageData->bytes(), width, height);
                   }
               }
           }

           std::shared_ptr<STI::Utils::FileHolder> imageFile;
           if (image->getFile(imageFile) && imageFile != nullptr) {
               processImageFile(imageFile->getID());
           }
       }
   }

``BinaryData::getBytes()`` also materializes a lazy stream if one is attached,
so existing byte-oriented code continues to work.  Prefer calling
``materialize()`` first when the code needs to distinguish transfer failure from
normal empty data.

Code that wants to copy a payload into another storage target can call
``BinaryData::transferTo()`` with a ``BinaryDataStreamTarget``.  This streams
from either local data or a lazy source without exposing network implementation
details.

C++ device output
+++++++++++++++++

Device authors publish binary and image measurements by setting a
``MixedValue`` to the public utility objects.  No special lazy API is required
on the producing side.

.. code-block:: c++

   #include <algorithm>
   #include <memory>
   #include <vector>

   #include <sti/utils/BinaryData.h>
   #include <sti/utils/Image.h>
   #include <sti/utils/MixedValue.h>

   bool CameraDevice::readChannel(short channel,
                                  const STI::Utils::MixedValue& value,
                                  STI::Utils::MixedValue& data)
   {
       if (channel == 0) {
           std::vector<char> frame = camera.readRawFrame();

           auto binary = std::make_shared<STI::Utils::BinaryData>();
           char* bytes = binary->allocate<char>(frame.size());
           std::copy(frame.begin(), frame.end(), bytes);

           data.setValue(binary);
           return true;
       }

       if (channel == 1) {
           std::vector<char> frame = camera.readRawFrame();

           auto binary = std::make_shared<STI::Utils::BinaryData>();
           char* bytes = binary->allocate<char>(frame.size());
           std::copy(frame.begin(), frame.end(), bytes);

           auto image = std::make_shared<STI::Utils::Image>();
           image->setWidth(camera.width()).setHeight(camera.height());
           image->setImageData(binary);

           data.setValue(image);
           return true;
       }

       return false;
   }

When these measurements are returned across the network by ``read()`` or become
remote channel state, the network layer sends them as lazy references.

Python client access
++++++++++++++++++++

Python clients use the same ``MixedValue`` entry points.  ``getBinary()`` and
``getImage()`` return wrapper objects that expose explicit pull and save
methods.

For a direct remote read, binary channels return ``BinaryData`` and image
channels return ``Image``.  The returned object exposes metadata before the
payload is pulled.

.. code-block:: py

   binary = device.read(0)

   print(binary.bytes(), binary.length(), binary.wordsize())
   print(binary.hasStream(), binary.isMaterialized())

   if binary.pull():
       payload = binary.getBytes()
       binary.save("results/frame.bin")

   image = device.read(1)

   print(image.getWidth(), image.getHeight(), image.getFileID())

   if image.hasData():
       data = image.getData()
       print(data.bytes(), data.hasLocalData())
       if data.pull():
           pixels = data.getBytes()

For cached channel measurements, access the ``MixedValue`` first and then pull
the embedded payload explicitly.

.. code-block:: py

   measurement = channel.getLastMeasurement()

   if measurement.getType() == stipy.MixedValueType.Binary:
       binary = measurement.getBinary()

       print(binary.bytes(), binary.length(), binary.wordsize())
       print(binary.hasStream(), binary.isMaterialized())

       if binary.pull():
           payload = binary.getBytes()
           binary.save("results/frame.bin")

For images, use ``getImage()`` and then pull the embedded binary data when the
image contains inline bytes.

.. code-block:: py

   measurement = channel.getLastMeasurement()

   if measurement.getType() == stipy.MixedValueType.Image:
       image = measurement.getImage()

       print(image.getWidth(), image.getHeight(), image.getFileID())

       if image.hasData():
           data = image.getData()
           if data.pull():
               pixels = data.getBytes()

       saved = image.save("results/frame.img")

``BinaryData.getBytes()``, ``BinaryData.save()``, ``Image.pullData()``, and
``Image.save()`` pull inline lazy data as needed.  ``Image.hasFile()`` reports
whether the image already has an accessible file holder; ``Image.getFileID()``
reports file identity metadata.  ``MixedValue.getValue()`` returns Python
``bytes`` for binary mixed values, which materializes the payload implicitly.
Use direct read return objects or ``getBinary()`` when the code needs to
inspect metadata or control when the transfer occurs.

Python device output
++++++++++++++++++++

Python device code can publish binary measurements with ``stipy.BinaryData``
and image measurements with ``stipy.Image``.

.. code-block:: py

   def readChannel(self, channel, value):
       if channel == 0:
           payload = self.camera.read_raw_frame()
           return stipy.BinaryData(payload)
       if channel == 1:
           payload = self.camera.read_raw_frame()
           return stipy.Image(stipy.BinaryData(payload), width=10, height=10)
       if channel == 2:
           payload = self.camera.read_raw_frame()
           file_holder = self.makeVirtualFileHolder("", "frame.raw")
           if file_holder is None or not file_holder.openFile():
               return None
           try:
               if not file_holder.writeBytes(payload):
                   return None
           finally:
               file_holder.closeFile()
           return stipy.Image(file_holder, width=10, height=10)
       return None

``stipy.BinaryData`` expects a Python ``bytes`` object and stores it with
``wordsize() == 1``.  ``stipy.Image`` can be constructed from Python ``bytes``,
``BinaryData``, ``FileHolder``, or ``FileID`` objects.

Shot results and persistence
++++++++++++++++++++++++++++

Live channel state is allowed to contain lazy remote references.  Completed shot
results are not.  During result collection, the server pulls lazy binary and
image measurement data and rewrites it to server-local file or image
references.  Code that reads archived ``ShotResult`` measurements should not
depend on the original producing device still being online.

For C++ persistence code that needs to copy an image into a durable holder, use
``Image::write(sourceFileServer, destinationFileHolder)``.  It handles both
file-backed images and inline ``BinaryData`` images, including lazy binary data
that must be pulled before writing.

Example map
***********

Use these examples as starting points for specific device features:

.. list-table::
   :header-rows: 1

   * - Example
     - Feature
   * - ``simpleDevice``
     - minimum ``LocalDevice`` and ``NetworkDeviceHub``
   * - ``configFile``
     - loading device and hub configuration from a file
   * - ``readWrite``
     - channel definitions, metadata, ``writeChannel``, and ``readChannel``
   * - ``attributes``
     - attribute setters, refreshers, allowed values, and metadata
   * - ``parseEvents``
     - ``RawEvent`` parsing, custom ``SynchronousEvent`` classes, parse errors
   * - ``partnerEvents``
     - event targets and partner-device timing
   * - ``listeners``
     - message receiver callbacks
   * - ``monitors``
     - manual monitors and automatic monitor updates
   * - ``tasks``
     - interval, appointment, and custom tasks
   * - ``logging``
     - device logs and recurring log tasks
   * - ``profiles``
     - saving and loading channel and attribute state
   * - ``fileMeasurement``
     - storing file-oriented measurement data
