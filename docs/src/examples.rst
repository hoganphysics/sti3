.. _deviceexamples:

===============
Device Examples
===============

Example device drivers live under ``examples/``.  The maintained examples are
the C++ examples in ``examples/cpp`` and the Python examples in
``examples/python``.  Each directory focuses on one device-library feature and
is intended to be small enough to copy when starting a new driver.

Most examples define a ``TestDevice`` or ``SimpleDevice`` class, create a
``NetworkDeviceHub``, attach the device, and run the hub.  The Python examples
also include notebooks for interactive testing where useful.

Recommended Starting Points
---------------------------

.. list-table::
   :header-rows: 1

   * - Example
     - Languages
     - What it demonstrates
   * - ``simpleDevice``
     - C++, Python
     - The minimum ``LocalDevice`` and ``NetworkDeviceHub`` application.
   * - ``configFile``
     - C++, Python
     - Reading device and hub configuration from an INI file.
   * - ``readWrite``
     - C++, Python
     - Declaring channels and implementing ``writeChannel`` and ``readChannel``.
   * - ``fileTransfer``
     - C++, Python
     - File and image channel values, lazy payload reads, and ``FileID`` import
       for ``device.read()`` / ``device.write()`` arguments.
   * - ``attributes``
     - C++, Python
     - Attribute setters, refreshers, allowed values, and metadata.
   * - ``parseEvents``
     - C++, Python
     - Converting STIPy ``RawEvent`` objects into device-specific
       ``SynchronousEvent`` objects.
   * - ``listeners``
     - C++, Python
     - Subscribing to device messages such as channel, attribute, and engine
       updates.
   * - ``logging``
     - C++, Python
     - Writing named logs and scheduling recurring log tasks.
   * - ``tasks``
     - C++, Python
     - Interval tasks, appointment tasks, and custom background tasks.
   * - ``postProcess``
     - C++, Python
     - Registering post-processing targets that run analysis after a shot plays.
   * - ``profiles``
     - C++
     - Saving and loading channel and attribute state with the profile manager.
   * - ``monitors``
     - Python
     - Manual monitors and automatic monitor updates for live device status.
   * - ``partnerEvents``
     - Python
     - Generating timing events for partner devices.
   * - ``fileMeasurement``
     - C++
     - Producing measurement data backed by files.

C++ Examples
------------

C++ examples are located in ``examples/cpp/<example>/src``.  They include
``CMakeLists.txt`` files and are the best templates for standalone C++ device
executables.

Typical files:

* ``main.cpp`` creates the configuration, device, and ``NetworkDeviceHub``.
* ``TestDevice.h`` declares the ``LocalDevice`` subclass.
* ``TestDevice.cpp`` implements the feature being demonstrated.

For example, ``examples/cpp/readWrite`` shows the common structure for a device
with output channels, input channels, and input channels that accept read
arguments.  ``examples/cpp/fileTransfer`` shows file and image channel values
and imported ``FileID`` arguments.  ``examples/cpp/parseEvents`` is the best
starting point for a
hardware-timed device because it includes custom ``SynchronousEvent`` classes
and parse-time error handling.

Python Examples
---------------

Python examples are located in ``examples/python/<example>``.  Most examples
include a ``testDevice.py`` file containing the ``LocalDevice`` subclass and a
``main.py`` or notebook that runs the device or exercises the API.

Use these when building with ``stipy.stidevicepy``:

* ``simpleDevice`` for the smallest network-connected device.
* ``readWrite`` for channel metadata and Python ``readChannel`` /
  ``writeChannel`` hooks.  ``readWrite/image_read.ipynb`` is the shortest
  notebook example for reading an image channel and converting it with Pillow.
* ``fileTransfer`` for file/image channel values, lazy reads, and imported
  ``FileID`` read/write arguments.
* ``parseEvents`` for custom Python ``SynchronousEvent`` classes.
* ``tasks`` and ``logging`` for recurring background work.
* ``monitors`` for live status values shown through the monitor manager.
* ``postProcess`` for registering analysis targets that run after a shot plays.
  See :ref:`stipypostprocessing` for the timing-file side that requests them.

How To Use The Examples
-----------------------

Start with the example that matches the feature you need, then copy the device
class into a new project and replace the simulated hardware calls with real
driver calls.  Keep the example's channel, attribute, and parser structure until
the new hardware behavior is working; this makes it easier to compare your
driver with the known working pattern.

The examples assume an STI network with an omniORB name service and a target
server.  For local development, update the ``Device Name``, ``IP Address``,
``Module``, ``Target Server``, and name-service address in the example config or
main file to match your test network.
