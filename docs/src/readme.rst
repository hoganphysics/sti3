###
STI
###

**Stanford Timing Interface**

**STI** is a library for building modular, distributed, hard-timing 
data acquisition and control systems for physics experiments.
It serves as an interface to translate high-level, human readable timing 
instructions into the equivalent low-level hardware implementation.

.. 
   It provides documentation of data

.. 
   Individual sub-systems can be run independenly in parallel or globally

*Modular:* The STI library defines a :ref:`device <devicelib>` abstraction allowing 
each hardware component to declare a set of I/O channels that can be controlled 
using a hardware-agnostic python interface, :ref:`STIPy <libstipy>`.
This allows new hardware drivers to be added in a modular fashion, and keeps
all hardware-specific implementation details separate from the timing control logic.


*Distributed:* STI supports distributed hardware control over a local area network. 
Devices communicate with each other over TCP/IP using a hierarchical 
:ref:`network <devicenetwork>`, allowing for flexible, parallel control of sub-systems.


*Hard-timing:* :ref:`Hard-timing <hardtiming>` support is enabled by appropriate 
hardware, such as with FPGA-controlled DAQ systems. The STI library provides the 
software interface to configure a hardware- or firmware-based event table with 
the desired timing sequence, which can then be triggered. For non-timing-critical 
applications, STI also allows for software control (soft timing) of hardware, which 
may run in parallel with the hard-timing components.
