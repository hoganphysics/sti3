.. _libstipy:

=====
STIPy
=====

Introduction
------------

STIPy is the timing interface language for STI.  It is a python library that 
allows timing sequences to be specified as high-level, human-readable scripts 
("timing files").  These timing sequences are submitted to the STI device
network where they are complied into low-level, device-specific commands that 
can then be executed on hardware.


STI timing files specify a sequence of **events** that occur on the 
**channels** that are available on the STI network. Each event is defined by its time, 
target channel, and value, corresponding to when, where, and what.  
The time determines when the event occurs.  The target specifies a 
:ref:`device <devicelib>` and a specific channel on that device where 
the event will occur.  The value parameter determines what the device's channel 
will do when the event occurs (e.g., it is the channel's new output value).

.. _hardtiming:

Hard timing vs Soft timing
++++++++++++++++++++++++++

STIPy supports both hard timing and soft timing devices.  Here *hard timing* 
generally refers to an event sequence that uses **hardware** control, with a 
timing precision better than can 
be realized by typical software control. The timing edges in a hard timing sequence 
are deterministic and are set by hardware or firmware, rather than software.  
One example of this is an FPGA-based 
timing system, which typically supports nanosecond (or better) timing jitter on event 
transitions. To realize hard timing control, the event sequence is precompiled and 
loaded into an event table on the hard timing device (e.g., an FPGA), with the 
appropriate time delays and channel values encoded in the table. Once triggered, 
the events are then played deterministically. In this application, STIPy acts as 
a high-level interface to help create and load event tables onto timing-critical 
systems.

STIPy also allows control of so-called *soft timing* devices. Soft timing refers to 
event transitions that are controlled in **software**, and are therefore limited to the 
typical millisecond-level latency and jitter associated with the computer operating 
system.  This kind of software-control has worse timing precision and is less 
deterministic than hardware- or firmware-based approaches, but is usually easier to 
implement and can be appropriate for some non-critical applications.

The STI network allows for a mixture of both hard- and soft-timing devices in the 
same timing sequence. STIPy provides an abstraction that does not distinguish 
between these two kinds of devices, so timing sequences 
can specify events on any device using the same interface. The different 
behavior for hard- and soft-timing is implemented at the level of the individual 
:ref:`device drivers <devicelib>` in the STI network. A hard-timing device requires 
appropriate hardware to support some kind of a triggerable event table (e.g., FPGA), 
as well as a device driver capable of translating the events specified in STIPy into 
hardware-specific commands. See the :ref:`STI device library <devicelib>` for more 
details on developing hard-timing device drivers.



Setup
-----

Install STIPy with the STI3 conda package:

.. code-block:: bash

    conda create -n sti3 python=3.13
    conda activate sti3
    conda install -c conda-forge hoganlab::stipy

For an existing conda environment:

.. code-block:: bash

    conda activate sti3
    conda install -c conda-forge hoganlab::stipy

The conda package includes the Python package, the C++ STI libraries, public
headers, and examples.  See :doc:`setuptools` for full installation and C++
project setup details.

STIPy can be imported in Python using:

.. code-block:: py

    from stipy import *

STIPy can also be run from a Jupyter notebook. The notebook server must be using
the conda environment where ``stipy`` is installed.  To submit timing sequences
for parsing or playback, the notebook process also needs network access to the
STI device network.

Checking versions
+++++++++++++++++

STIPy exposes the STI package version directly from Python:

.. code-block:: py

    import stipy

    print(stipy.__version__)
    print(stipy.version())
    stipy.printVersion()

``stipy.__version__`` is the package version string.  ``stipy.version()``
returns a summary string with the STI library version, build number, and git
commit information when available.  ``stipy.printVersion()`` prints the same
summary and also returns it.

Connected devices expose their version records through the device
``VersionManager``.  This includes the STI library version used by the device
and any device-specific version records registered by the driver.

.. code-block:: py

    from stipy import *

    server = connect("localhost/0/STI Server", "192.168.1.4:2809")
    versions = server.getVersionManager()

    print(versions.summary())

    library = versions.getLibraryVersion()
    print(library.version, library.buildNumber, library.gitCommit)

    for info in versions.getVersions():
        print(info.component, info.version)


.. _stipytimingseqences:

Basic timing sequences  
----------------------


Targets
+++++++

Timing events must specify a **target**, which is the device and channel number 
where the event will occur.  Any STI device connected to the network is a valid 
target for events. To specify a target device, use the ``dev()`` function:

.. code-block:: py

    dev(name, address, module)


Here ``name``, ``address``, and ``module`` are the elements of the target device's 
:ref:`DeviceID` needed to fully specify the device on the STI network. The ``name``
and ``address`` arguments must be strings, and the ``module`` argument is an integer.
The ``dev()`` function returns a ``TargetDevice`` object which must be used when 
creating events on this device. 

In addition to the target device, the channel number of the event must also be 
specified. This is done with the help of the ``ch()`` function:

.. code-block:: py

    ch(deviceTarget, channel)

Here ``deviceTarget`` is a ``TargetDevice`` instance (created by ``dev()``, for 
example), and ``channel`` is the integer channel number. 
Devices declare their own channel lists when connecting to the network, so ``channel`` 
may be any number supported by the target device. 
The ``ch()`` function returns a ``TargetChannel`` object which contains the 
``TargetDevice`` reference and the channel number.




It is also possible to define *abstract device targets*:

.. code-block:: py

    dev(abstractName)   #abstract device

where ``abstractName`` is the string name of the abstract device. An abstract 
``TargetDevice`` instance does not point to any specific DeviceID on the network, 
but instead stands in as a placeholder. Abstract 
targets allow the logical structure of a timing file to be written without 
reference to specific DeviceIDs. This allows the same timing files to be reused 
on other STI networks with different hardware targets. 
All abstract targets must be replaced with fully specificed concrete target devices 
before they can be run on the network. This is accomplished by specifying a 
dictionary that maps the abstract targets to concrete ``TargetDevice`` instances.

Likewise, *abstract channel targets* are also allowed:

.. code-block:: py

    ch(abstractName)                 #pure abstract channel
    ch(deviceTarget, abstractName)   #abstract channel on deviceTarget

As above, ``abstractName`` a string defining the name of the abstract channel. 
The abstract channel can be defined purely in terms of a name, or it may be 
specified with respect to some ``TargetDevice`` instance ``deviceTarget``, which 
iself can be concrete or abstract.



Events
++++++

The are two basic types of events are **input events** and **output events**.  
An output event is an event that changes the output value of some channel. For example, 
changing the analog voltage of a DAC or the frequency of a DDS are examples of output 
events.  Output events are declared with the ``event()`` function:

.. code-block:: py

    event(channel, time, value, [group])    #output event

Here ``channel`` is a ``TargetChannel`` object, ``time`` is a the floating point time 
of the event (in nanoseconds), and ``value`` is the desired output value of the 
channel.  The ``value`` field can have any basic type (float, string, list, etc), as 
required by the ``TargetChannel`` of the event. The interpretation and use of the 
``value`` field for any given channel is specified in the implementation the
:ref:`device driver <devicelib>` for that channel. The optional ``group`` field 
is a scoping construct that allows events to be grouped together in a hierarchical 
structure for various purposes. See the :ref:`stipygroups` section for more information.


An input events is an event that makes a measurement on an input channel and stores 
the measured value. All data collected during a timing sequence is the result of 
input events.  Examples include measuring an analog voltage with an ADC or taking a 
picture with a CMOS camera. Input events are declared with the ``meas()`` function:

.. code-block:: py

    meas(channel, time, [group])            #input event
    meas(channel, time, value, [group])     #input event with value command

Here ``channel`` is the ``TargetChannel`` of the measurement, ``time`` is the floating 
point time of the measurement. As above, input events can optionally be added to a 
group with the ``group`` field. A second form of ``meas()`` is also available that 
accepts a ``value`` field, similar to the ``event()`` function.  The ``value`` field 
may be used to specify any needed parameters of the measurement, or alternatively to 
realize hybrid input/output channel functionality. As always, the details of how a 
channel interprets the ``value`` field depends on the device's implementation.


Running shots
-------------

Connecting to a parser/player
+++++++++++++++++++++++++++++

Parsing and playing require a device reference with access to an
``EventEngineScheduler``.  Use ``connect()`` to create a local STIPy client
device and connect it to a device on the STI network:

.. code-block:: py

    from stipy import *

    server = connect("localhost/0/STI Server", "192.168.1.4:2809")

The first argument is the target device's ``DeviceID``.  The second argument is
the omniORB name service address.  The target device does not have to be named
``STI Server``.  Any STI device can be connected to, and any connected device
can be used as the parser/player as long as that device has references to all
devices targeted by the shot's event list.  This is useful for small tests, such
as parsing and playing a shot that only targets a single device:

.. code-block:: py

    test_device = connect("localhost/0/TestDevice", "192.168.1.4:2809")

If a configuration file contains the name service address in
``NetworkHub/NameService``, pass the config and omit the second argument:

.. code-block:: py

    config = ConfigFile("stipy.ini")
    server = connect("localhost/0/STI Server", config=config)

If the server device is hosted by a hub whose ``HubID`` is not the default hub
derived from the device ID, pass ``serverHubID``:

.. code-block:: py

    server = connect(
        "localhost/0/STI Server",
        "192.168.1.4:2809",
        serverHubID="localhost/0/STI Server Hub",
    )

Building a shot
+++++++++++++++

The usual pattern is to write a Python function that declares events, then pass
that function to ``makeshot()`` or ``server.makeshot()``:

.. code-block:: py

    camera = dev("Camera", "localhost", 0)
    shutter = ch(camera, 0)
    image = ch(camera, 1)

    def shotmaker():
        event(shutter, 0, True)
        meas(image, 10_000_000)
        event(shutter, 20_000_000, False)

    shot = makeshot(shotmaker)

``makeshot`` executes ``shotmaker`` in a shot-building context.  Global calls
such as ``event()``, ``meas()``, ``setvar()``, ``settag()``, and ``group()`` add
data to the shot currently being built.  These global helpers must be called
inside a ``makeshot`` call.

The global ``makeshot()`` form builds a local shot without requiring a connected
server.  This is useful for constructing or inspecting a shot before deciding
where to parse it.  Parsing and playing still require a connected server:

.. code-block:: py

    shot = makeshot(shotmaker)
    parse_ticket = server.parse(shot)

``server.makeshot()`` accepts the same Python arguments, but creates the shot
through the connected server interface and records server-related shot
configuration such as the submitting host and user.

The first argument is called ``source``.  It may be omitted, a callable, or a
Python filename:

.. code-block:: py

    empty_shot = makeshot()
    function_shot = makeshot(shotmaker)
    file_shot = makeshot("timing_files/mot_load.py")

    server_empty_shot = server.makeshot()
    server_function_shot = server.makeshot(shotmaker)
    server_file_shot = server.makeshot("timing_files/mot_load.py")

``makeshot`` can also take a Python filename.  In this form STIPy loads and
executes the file while the shot-building context is active, so top-level calls
to ``event()``, ``meas()``, ``setvar()``, ``settag()``, and ``group()`` in that
file become part of the shot.

.. code-block:: py

    shot = makeshot("timing_files/mot_load.py")

For example, ``timing_files/mot_load.py`` might contain:

.. code-block:: py

    from stipy import *

    test_device = dev("TestDevice", "localhost", 0)
    output0 = ch(test_device, 0)
    input10 = ch(test_device, 10)

    settag("MOT load")
    setvar("bias", 2.5)
    event(output0, 0, var("bias"))
    meas(input10, 1_000_000)
    event(output0, 2_000_000, 0.0)

This is useful when timing files are maintained as standalone Python scripts.
The file's directory is added to Python's import path before execution, so the
timing file can import helper modules located beside it.

File-backed shots use isolated timing imports.  Each call executes the submitted
file and helper modules under that file's directory from a fresh import state,
then removes those timing modules from ``sys.modules`` when the shot is built.
This lets repeated ``makeshot("timing_files/mot_load.py")`` calls pick up
top-level ``setvar()``, ``settag()``, and event declarations from helper files
without restarting Python.  Stable modules such as ``stipy``, standard-library
modules, and installed packages are shared normally.

If timing helpers live outside the submitted file's directory, pass their
directories with ``import_roots``:

.. code-block:: py

    shot = makeshot(
        "timing_files/mot_load.py",
        import_roots=["timing_files", "shared_timing"],
    )
    shot = server.makeshot(
        "timing_files/mot_load.py",
        import_roots=["timing_files", "shared_timing"],
    )

The submitted file is executed in private module globals, so variables assigned
at top level in the timing file do not become globals in the calling notebook or
script.

Variable overrides can be supplied when making a shot from either a function or
a file.  Pass a dictionary from variable name to value, or a set of
``ParsedVar`` objects:

.. code-block:: py

    shot = makeshot(shotmaker, vars={"bias": 3.0})
    shot = makeshot("timing_files/mot_load.py", vars={"bias": 3.0})

    shot = server.makeshot(shotmaker, vars={"bias": 3.0})
    shot = server.makeshot("timing_files/mot_load.py", vars={"bias": 3.0})

By default, shots are created with ``ShotType.Single``.  Use ``shot_type`` when
building shots for sequence entries or other scheduler-specific contexts:

.. code-block:: py

    shot = makeshot(shotmaker, vars={"bias": 3.0}, shot_type=ShotType.SequenceEntry)
    shot = server.makeshot(shotmaker, vars={"bias": 3.0}, shot_type=ShotType.SequenceEntry)

You can also create an empty shot and add to it directly:

.. code-block:: py

    shot = server.makeshot()
    shot.event(shutter, 0, True)
    shot.meas(image, 10_000_000)

Parsing
+++++++

Parsing validates the target devices and channels, resolves variables and
groups, and asks each target device to convert raw timing events into
device-specific synchronous events.

.. code-block:: py

    parse_ticket = server.parse(shot)
    parse_ticket.wait()

    for message in parse_ticket.messages():
        print(message)

``parse()`` returns a parse ticket immediately.  Call ``wait()`` when the code
needs the final parse status before continuing.

Playing
+++++++

After a shot has parsed successfully, submit the parse ticket to ``play()``:

.. code-block:: py

    result_ticket = server.play(parse_ticket)
    result_ticket.wait()

    print(result_ticket.measurements())

The result ticket can be used to inspect the shot ID, status, and measurement
results.  Shot data is also available later through the connected device's
``PersistenceManager``.

Binary and image measurements
+++++++++++++++++++++++++++++

Remote reads and remote channel state can expose large binary and image
measurements as lazy values.  Direct binary reads return ``BinaryData`` and
direct image reads return ``Image`` so Python code can inspect metadata,
explicitly pull bytes, or save the payload.  Use ``MixedValue.getBinary()`` or
``MixedValue.getImage()`` for cached channel measurements without relying on
implicit conversion through ``MixedValue.getValue()``.  See
:ref:`lazy_payloads` for the full C++ and Python API.

Complete example
++++++++++++++++

.. code-block:: py

    from stipy import *

    server = connect("localhost/0/STI Server", "192.168.1.4:2809")

    test_device = dev("TestDevice", "localhost", 0)
    output0 = ch(test_device, 0)
    input10 = ch(test_device, 10)

    def single_shot():
        settag("Example")
        setvar("bias", 2.5)

        event(output0, 0, var("bias"))
        meas(input10, 1_000_000)
        event(output0, 2_000_000, 0.0)

    shot = server.makeshot(single_shot)
    parse_ticket = server.parse(shot)
    parse_ticket.wait()

    result_ticket = server.play(parse_ticket)
    result_ticket.wait()
    print(result_ticket.measurements())


Variables
---------

Variables name values used while a shot is built.  They are recorded in the
shot's event group data, which makes the shot easier to inspect and allows
sequence entries to override selected values.

.. code-block:: py

    setvar("bias", 2.5)
    event(output0, 0, var("bias"))

Variables can be scoped to a group:

.. code-block:: py

    setvar("detuning", -12.0, "MOT")
    event(output0, 10_000, var("MOT/detuning"))

    mot = group("MOT")
    mot.setvar("duration", 20_000_000)
    event(output0, mot.var("duration"), 0.0, mot)

When a variable is bound, ``var("name")`` returns its value for normal Python
use.  When a variable is declared as an overwritten variable for a sequence,
``var("name")`` can remain symbolic while the sequence entry supplies the value.

.. _stipygroups:

Groups
------

Groups are a scoping construct for timing files.  They provide namespace-like
isolation for events, variables, and tags, which helps reuse code without name
collisions and makes generated event tables easier to inspect.  Grouped events
can be viewed together in the event table, so a timing sequence can be reasoned
about in named pieces such as ``MOT``, ``Imaging``, or ``Cleanup``.

A group can contain:

* events and measurements
* variables
* tags
* subgroups
* metadata such as display color

Creating groups
+++++++++++++++

.. code-block:: py

    mot = group("MOT")
    imaging = group("Imaging", color="blue")

Nested groups use ``/`` in their full names or ``group()`` on an existing group:

.. code-block:: py

    shutter = group("Imaging/Shutter")

    imaging = group("Imaging")
    shutter = imaging.group("Shutter")

Adding events and measurements to groups
++++++++++++++++++++++++++++++++++++++++

There are several equivalent ways to add events to a group.

Pass the group name to the global helper:

.. code-block:: py

    event(output0, 0, True, "MOT")
    meas(input10, 5_000_000, group="MOT")

Use the group object:

.. code-block:: py

    mot = group("MOT")
    mot.event(output0, 0, True)
    meas(input10, 5_000_000, group=mot.getFullName())

Use the shot object when building manually:

.. code-block:: py

    shot = server.makeshot()
    shot.event(output0, 0, True, "MOT")
    shot.meas(input10, 5_000_000, group="MOT")

For measurements with command arguments, pass the value before the group:

.. code-block:: py

    meas(input10, 5_000_000, [0.1, "fast"], "MOT")
    mot.meas(input10, 5_000_000, [0.1, "fast"])

Group variables
+++++++++++++++

Variables inside a group are referenced by their full group path:

.. code-block:: py

    group("MOT").setvar("load_time", 25_000_000)
    event(output0, var("MOT/load_time"), 0.0)

These forms are equivalent:

.. code-block:: py

    mot = group("MOT")
    mot.setvar("load_time", 25_000_000)

    setvar("load_time", 25_000_000, "MOT")
    setvar("MOT/load_time", 25_000_000)

Tags
++++

Tags mark named points or regions in a shot.  They are recorded with stack trace
information and group scope, making timing files easier to inspect and document.
Use tags for labels such as ``MOT load``, ``Probe``, ``Camera exposure``, or
``Cleanup``.

Set a tag in the root group:

.. code-block:: py

    settag("MOT load")

Set a tag in a named group:

.. code-block:: py

    settag("Open shutter", "Imaging")

Use the group object:

.. code-block:: py

    imaging = group("Imaging")
    imaging.settag("Open shutter")

Like variables, tags can also use full group paths:

.. code-block:: py

    settag("Imaging/Open shutter")

Reusable grouped code
+++++++++++++++++++++

Groups are especially useful when a helper function emits a reusable block of
timing events.  The helper can take a group name and keep its variables, tags,
and events isolated from other uses of the same helper.

.. code-block:: py

    def pulse_block(name, target, start, duration, amplitude):
        g = group(name)
        g.settag("start")
        g.setvar("amplitude", amplitude)
        g.event(target, start, g.var("amplitude"))
        g.event(target, start + duration, 0.0)
        g.settag("end")

    def shotmaker():
        pulse_block("MOT/Coils", output0, 0, 10_000_000, 2.5)
        pulse_block("Imaging/Coils", output0, 20_000_000, 2_000_000, 1.0)

Inspecting groups
+++++++++++++++++

After making a shot, inspect the root group and subgroups:

.. code-block:: py

    shot = server.makeshot(shotmaker)
    root = shot.rootgroup()

    print(root.getTotalStats())
    for node in root.flatten():
        print(node.name(), node.startTime(), node.endTime(), node.stats())

Group inspection is useful for checking that a timing helper produced the
expected events, variables, and tags before parsing the shot.

.. _stipypostprocessing:

Post-processing
---------------

Post-processing runs analysis code after a shot finishes playing, without
blocking the parsing or playback of later shots.  A typical use is a lightweight
"analysis" device on the STI network that fits or reduces a shot's measurement
data (for example, counting atoms from an absorption image or fitting a
resonance scan) on a background worker thread, then broadcasts the results so
other clients can subscribe to them.

A post-processing **target** is a named analysis routine registered on a device.
See :ref:`devicepostprocessing` for how a device author registers a target.  A
timing file requests post-processing against a target much like it declares an
event target, but with no ``time`` argument: a ``postProcess()`` request is
deliberately **not** hard-timed.

Post-processing targets
+++++++++++++++++++++++

Name a target with ``postTarget()``:

.. code-block:: py

    postTarget(device, name)

``device`` is the analysis device (a device name string, or a ``TargetDevice``
from ``dev()``) and ``name`` is the registered target name on that device.
``postTarget()`` returns a ``PostProcessTarget`` object.  Like ``dev()`` and
``ch()``, it works with no live connection, so abstract, name-only targets can
be written in a standalone timing file:

.. code-block:: py

    analysis = postTarget("Analysis", "atom number")

Requesting post-processing
++++++++++++++++++++++++++

Use ``postProcess()`` inside a shot to request that a target process the shot
when it finishes playing:

.. code-block:: py

    postProcess(target, options=None)

``target`` is a ``PostProcessTarget``.  ``options`` is an optional dictionary of
named parameters passed to the analysis routine, such as a region of interest or
a fit model.  Note the deliberate absence of a ``time`` argument: this is the
visible signal, right in the call shape, that a ``postProcess()`` request is not
hard-timed, unlike ``event(target, time, value)`` and
``meas(target, time, value)``.

.. code-block:: py

    camera = dev("Camera", "localhost", 0)
    shutter = ch(camera, 0)
    image = ch(camera, 1)

    def shotmaker():
        event(shutter, 0, True)
        meas(image, 10_000_000)
        event(shutter, 20_000_000, False)

        # Run analysis after the shot plays; not hard-timed, so no time argument.
        postProcess(postTarget("Analysis", "atom number"),
                    {"roi": [10, 20, 100, 100], "model": "gaussian"})

    shot = makeshot(shotmaker)

Like ``event()`` and ``meas()``, ``postProcess()`` is a global helper that
applies to the shot currently being built, and there are equivalent forms on the
shot and group objects:

.. code-block:: py

    shot = server.makeshot()
    shot.postProcess(postTarget("Analysis", "atom number"))

    mot = group("MOT")
    mot.postProcess(postTarget("Analysis", "MOT fit"), {"roi": [0, 0, 50, 50]})

More than one request may target the same analysis device in a single shot, and
the same target may be requested more than once; each request is queued and
processed independently.

A request whose target device cannot be found on the network produces a
non-fatal parse **warning** and is skipped; the shot still parses and plays
normally.  This is different from a missing hard-timed event target, which makes
the shot abstract and blocks playback.

.. note::

   An abstract, name-only ``postTarget("Analysis", ...)`` is convenient for
   authoring a standalone timing file, but post-process targets are only
   dispatched once they are bound to a concrete device.  Concretizing the
   post-process side-list from an abstract-to-concrete target dictionary is still
   in progress, so for now author the target concretely (for example
   ``postTarget(dev("Analysis", "192.168.1.4", 0), "atom number")``) to have it
   dispatched.  A target still abstract at parse time takes the warn-and-skip path
   above.

How results are delivered
+++++++++++++++++++++++++

When the shot finishes playing, the owning server dispatches each resolved
request along the shot's dependency tree to its target device, forwarding through
intermediate servers so a target nested behind a sub-server is still reached.
The target's background worker pulls the shot's ``ShotResult`` from the owning
device and passes it to the registered routine, which returns results that are
broadcast in a ``PostProcessingComplete`` device message (or an error message if
the routine raised, the owning device was unreachable, or the shot result was not
found).  On the device side the callback therefore receives ``(shot_result,
options)`` — already loaded — rather than a ``ShotID`` it must look up itself
(see :ref:`devicepostprocessing`).

Discovering targets
+++++++++++++++++++

A connected device reports its registered targets, each with the options it
understands:

.. code-block:: py

    analysis = connect("localhost/0/Analysis", "192.168.1.4:2809")

    for target in analysis.getPostProcessingTargets():
        print(target.name, "-", target.description)
        for option in target.options:
            print("   ", option.name, "-", option.description)

``getPostProcessingTargets()`` returns a list of ``PostProcessingTargetInfo``
objects, each with ``.name``, ``.description``, and ``.options`` (a list of
``PostProcessingOptionInfo`` with ``.name`` and ``.description``).  The option
list is the author-declared set of parameters a target accepts in its
``postProcess()`` payload; it is documentation only and is not validated.  This is
the live, network-resolved companion to writing ``postTarget(name, ...)`` in a
standalone timing file: a script can be authored against named targets offline,
and an interactive session or frontend can list the targets -- and supported
options -- a device actually offers.

Inspecting requests
+++++++++++++++++++

Post-processing requests are carried alongside the shot's event group as
metadata, not in the hard-timed event table.  Inspect them after building a
shot:

.. code-block:: py

    shot = makeshot(shotmaker)
    for request in shot.rootgroup().postProcessRequests():
        print(request.target().name(), request.options())

``request.options()`` returns the options dictionary supplied at the
``postProcess()`` call site.
