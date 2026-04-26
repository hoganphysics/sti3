========================
STI Library Architecture
========================

STI3 is built around a C++ device and network core with Python bindings for
device drivers, STIPy timing scripts, notebooks, and user-facing tools.

.. The legacy architecture diagram has been archived at
.. ``docs/figs/archive/architecture.png``.  The current maintained stack is:

.. list-table::
   :header-rows: 1
   :widths: 20 30 50

   * - Layer
     - Main technology
     - Role
   * - Device core
     - C++20
     - Device interfaces, channels, attributes, monitors, logs, profiles,
       tasks, persistence, and event parsing.
   * - Network core
     - C++20, CORBA, omniORB
     - Remote device references, hubs, message distribution, and RPC between
       devices.
   * - Python bindings
     - pybind11
     - ``stipy``, ``stipy.stidevicepy``, Python device drivers, and notebook
       workflows.
   * - Timing interface
     - STIPy
     - Python API for building shots, variables, tags, groups, sequences, and
       parse/play requests.
   * - Interactive front end
     - JupyterLab extension, JupyterHub
     - A separately maintained JupyterLab extension deployed through
       JupyterHub.  It connects to STI through the Python bindings rather than
       through a separate web-console server.
   * - Packaging
     - conda
     - Distribution of the Python package, C++ libraries, headers, binaries,
       and examples.

.. graphviz::
   :caption: Maintained STI3 stack.
   :name: sti3-maintained-stack

   digraph sti3_stack {
       graph [rankdir=LR, bgcolor="transparent", pad="0.2", nodesep="0.55", ranksep="0.75"];
       node [shape=plain, fontname="DejaVu Sans"];
       edge [color="#666666", arrowsize="0.8"];

       cpp [label=<
         <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="8" COLOR="#3b3b3b">
           <TR><TD BGCOLOR="#e8f1ff"><B>C++</B></TD></TR>
           <TR><TD>stidevice</TD></TR>
           <TR><TD>stinetwork</TD></TR>
         </TABLE>
       >];

       corba [label=<
         <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="8" COLOR="#3b3b3b">
           <TR><TD BGCOLOR="#fff0d9"><B>RPC</B></TD></TR>
           <TR><TD>CORBA</TD></TR>
           <TR><TD>omniORB</TD></TR>
         </TABLE>
       >];

       pybind [label=<
         <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="8" COLOR="#3b3b3b">
           <TR><TD BGCOLOR="#e8ffe8"><B>Python</B></TD></TR>
           <TR><TD>pybind11</TD></TR>
           <TR><TD>stipy</TD></TR>
         </TABLE>
       >];

       tools [label=<
         <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="8" COLOR="#3b3b3b">
           <TR><TD BGCOLOR="#f4e8ff"><B>Tools</B></TD></TR>
           <TR><TD>Python drivers</TD></TR>
           <TR><TD>notebooks</TD></TR>
           <TR><TD>JupyterLab extension</TD></TR>
         </TABLE>
       >];

       deploy [label=<
         <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="8" COLOR="#3b3b3b">
           <TR><TD BGCOLOR="#eeeeee"><B>Deploy</B></TD></TR>
           <TR><TD>conda package</TD></TR>
           <TR><TD>JupyterHub</TD></TR>
         </TABLE>
       >];

       cpp -> corba [label="network calls"];
       cpp -> pybind [label="bindings"];
       pybind -> tools [label="API"];
       deploy -> pybind [label="installs"];
       deploy -> tools [label="hosts"];
   }

The Java wrapper and the old JavaScript web console are not part of the
maintained user-facing stack described here.


stidevice Core Library
======================

The ``stidevice`` library is the local C++ implementation of the STI device
model.  It provides the concrete services behind a ``LocalDevice``: channel and
attribute managers, monitors, logs, profiles, tasks, persistence, event
engines, and the message and collection infrastructure used to connect devices
together.  The network layer exposes many of these same interfaces remotely,
but the core behavior is implemented in ``stidevice``.

LocalDevice and Managers
------------------------

``LocalDevice`` is the composition root for a running device process.  It owns
the local ``DeviceID``, the device collection, message dispatcher and receiver,
channel and attribute managers, monitor manager, task manager, log manager,
profile manager, persistence manager, file server, and local event engine
scheduler.  Device implementations normally subclass or configure
``LocalDevice`` and then register the parts of the hardware interface they
provide: channels, attributes, monitors, background tasks, event engines,
partner devices, and event targets.

Most device features are exposed through small manager interfaces instead of
through one large device API.  The public ``Device`` interface returns managers
such as ``ChannelManager``, ``AttributeManager``, ``MonitorManager``,
``ProfileManager``, ``TaskManager``, ``LogManager``,
``PersistenceManager``, and ``EventEngineScheduler``.  The local managers own
the concrete state and callbacks, while remote device wrappers can expose the
same manager-style interface over the network.  This keeps application code
mostly independent of whether a device reference is local, remote, returned by
``connect()``, or discovered through another device's collection.

The manager model also makes state updates observable.  Channel writes,
attribute changes, monitor updates, engine state changes, and scheduler job
changes are surfaced both through direct manager calls and through the device
message system.

Message Passing
---------------

Device state changes are represented as typed device messages.  Messages cover
collection changes, channel and attribute updates, monitor values and status,
engine state, parser messages, scheduler messages, and engine job updates.  A
local feature manager or event engine generates messages through the
``MessageGenerator``/``DeviceMessageDispatcher`` path, and clients subscribe
through ``DeviceMessageReceiver``.

The receiver keeps listener groups by source ``DeviceID`` and message type.  As
devices appear or disappear from the local device collection, the receiver
installs or removes message handlers on the source device's dispatcher.  This
keeps subscriptions source-scoped: a client can listen to one device's channel
updates, another device's engine scheduler messages, or a mix of event types
without polling every manager directly.

Device Reference Distribution
-----------------------------

STI devices discover each other by distributing device references rather than
by using a single global registry.  A ``LocalDeviceHub`` is a specialized
``LocalHub<DeviceID, Device>``.  It stores locally owned device references and
connects to other local or remote hubs.  When hubs are connected, they exchange
references to their locally owned devices and offer those references to the
devices they manage.

The generic distribution mechanism is built from collectors and distributers:

* A ``Collector<ID, T>`` exposes a collection that can receive references.
* A ``Distributer<ID, T>`` owns a set of nodes and a set of collectors.
* When a node is added, removed, or refreshed, the distributer updates each
  collector's collection, while avoiding self-references.
* ``LocalHub`` uses this pattern to keep device collections populated as hubs
  connect, disconnect, refresh, and redistribute nodes.

This design allows any device to hold references to other devices it needs to
coordinate with.  A connected device reference can then be used like any other
device reference: read channels, inspect attributes, subscribe to messages, or
get an event engine scheduler.

Engine Scheduler
----------------

The local event engine scheduler is implemented by
``LocalEventEngineScheduler``.  It manages parse, play, and sequence jobs for
the event engines owned by a device and implements prioritized parallel
distributed scheduling:

* Prioritized: jobs are stored by ``EngineJobID`` and considered in priority
  order.  Play jobs are given first assignment priority when a free engine
  already contains the requested parse result.
* Parallel: multiple parse jobs can run at the same time when there are free
  engines and the configured ``EngineConflictPolicy`` allows the overlap.
* Distributed: scheduling uses local knowledge and device references.  A device
  can parse and play shots that include events for other devices by using the
  dependency parser, scheduler messages, and connected device references.
* Scheduling: parse and play requests enter a queue.  A scheduler thread wakes
  when jobs are added, completed, or canceled, then assigns runnable jobs to
  available engines.

The scheduler enforces the key engine rules: an engine only runs one job at a
time; a play job must use an engine that has already parsed the matching
``ParseID``; and conflict policy controls which parse/play combinations may
overlap across engines.  Completed parse and play jobs are retained in bounded
buffers so recent status and results can still be inspected through the device
interface.

Distributed parsing starts by building a dependency tree for the shot's event
targets.  The scheduler finds the devices referenced by the raw event group,
transfers timing files through virtual file servers when needed, coordinates
parse and play progress with scheduler messages, and stores parse, shot,
sequence, measurement, and file results through the persistence manager.

Event parsing is split between generic engine infrastructure and device-owned
parsing logic.  STIPy or another client submits raw timing events in a
``Shot``.  The scheduler creates local jobs and uses
``LocalEventEngineDependencyParser`` to discover which devices are needed for
the shot.  The dependency parser builds an ``EventEngineDependencyTree`` from
the raw event targets, using the local device collection to find the device
references that must participate.

Once a job is assigned, a ``LocalEventEngine`` parses the events for its device
by calling the device's ``DeviceEventParser`` implementation.  For a
``LocalDevice``, this is the ``parseEvents()`` hook that device authors
override to convert raw events into ``SynchronousEvent`` objects.  Parse-time
errors, warnings, and information are carried as ``EngineParsingMessage``
objects with stack trace data, so diagnostics can point back to the timing
code that produced the event.  During playback, engines collect
``EnginePlayingMessage`` objects, measurements, and file results for the final
shot record.

The scheduler and engines communicate progress through typed device messages:
engine state messages, engine job update messages, parser messages, and
engine scheduler messages.  These messages allow the initiating device and
dependent devices to coordinate parse completion, play readiness, play
completion, yielded work, and result transfer without requiring a single
central scheduler that knows the whole network.

Persistence and Results
-----------------------

``LocalPersistenceManager`` is responsible for turning parser and playback
output into durable or inspectable records.  It implements the public
``PersistenceManager`` interface and stores parse results, shot results,
sequence results, measurements, timing files, generated data files, and log
paths.  It also listens to engine scheduler messages so that completed work can
be buffered, saved, or delegated as shots and sequences progress.

The persistence layer separates result collection from result storage.  During
playback, a ``ResultsCollector`` accepts measurements, attributes, playing
messages, and files from the devices involved in the shot.  The persistence
manager then transfers those results into a ``FullShotResult`` and saves them
through a ``ShotRepository``.  The default repository writes serialized results
to disk, while the transient repository and bounded in-memory buffers make
recent parse, shot, and sequence results available for immediate inspection.

File handling is part of the same subsystem.  The persistence manager owns or
uses a ``FileServer``, creates ``FileHolder`` objects for local files, and can
create ``VirtualFileServer`` instances so timing files and generated data can
be transferred between devices during distributed parsing and playback.  This
lets a shot record include both structured results and associated files without
requiring every device to share the same local filesystem.
