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

STIPy is available on the public Python package repository PyPI. To install, run

.. tabs::

   .. tab:: Linux

    .. code-block:: bash

        python3 -m pip install stipy
        
   .. tab:: Windows

    .. code-block:: bash

        py -m pip install stipy



STIPy can be imported in python using

.. code-block:: py

    from stipy import *

STIPy can also be run from a Jupyter notebook. For this to work, the notebook 
server must have the STIPy package installed. In order to submit timing sequences, 
the Jupyter server also requires access to the to an STI network. This can be 
accomplished by running the notebook server on the same LAN as the STI device 
network, for example.


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
structure for various purposes. See the ::ref:`Groups` section for more information.


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

Make Shot
+++++++++

.. code-block:: py

    shot = makeshot()

Parse
+++++

.. code-block:: py

    parseticket = server.parse(shot)

Play
++++

.. code-block:: py

    resultticket = server.play(parseticket)


Variables
---------

.. code-block:: py

    setvar(name, value, [group])


.. code-block:: py

    var(name)


Groups
------

.. code-block:: py

    group(name, [color])


.. code-block:: py

    g = group("groupName")              #gets RawEventGroup object
    g.event(channel, time, value)       #adds event to this group
    
    event(channel, time, value, "groupName")     #equivalent to above
    event(channel, time, value, g)               #equivalent to above

.. code-block:: py

    group("groupName").setvar("varName", value)

    setvar("varName", value, "groupName")   #equivalent to above
    setvar("groupName/varName", value)      #equivalent to above

