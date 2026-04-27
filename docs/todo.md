## Todo list for STI documentation

* Upgrade Setup section to describe conda installation.  Right now `docs/src/setuptools.rst` explains how to build the STI library from source, either using docker or by hand. This description is out of date. It also describes building and installing stipy using a python wheel, which is no longer supported.  Now for building and distribution we use a conda build and upload to anaconda.  The new procedure is described in `conda/conda-build.md`.
  * Some of the notes in build for source might still be useful for an advanced section, so I don't want to completely delete this. However I think we should archieve this version of setuptools.rst for now and write a new version that focuses on the conda install process.
  * The setup section should tell the user how to setup a local conda env and install stipy from anaconda. This should be at the very top, and should have copy buttons to copy the install commands.
  * The setup section should explain that the stipy conda package contains both the python library and also the c++ binaries and header files, as well as code examples for both python and c++.
  * The setup section should explain how to setup a new c++ project (cmake or visual studio) that points to the stipy install location so the project can find headers and libraries.

* The Device interface section (`interface.rst`) is incomplete. 
  * The point of this section is to explain how a user of the device library can access the different features that the device provides. It is not a description for how to build a new device, but how to programatically control a device give a device reference.
  * There is a short description of each of the different managers that the device provides.  Some newer managers are missing and need to be added. Most of the descriptions of the feature are very short and incomplete (for example, Channels and Attributes).
  * We need to get better coverage of the different features that each manager provides. This includes adding more explanatory text and usage exampls in python and c++.  Support for Java is not a priority right now.

* The Creating a device section `device.rst` is incomplete. 
  * The point of this section is to explain how to implement a new device using the sti library. The focus is on the c++ and python binding.  Support for Java is not a priority right now.
  * The descriptions are incomplete. We need to get better coverage on the features for building devices.
  * There are many examples of devices in `examples/cpp/` and `examples/python` that we can use to explain different features. We need to add new section to `device.rst` for missing features, and give more code examples.
  * As a rough guide, each of the examples in the `examples` directory focuses on explaining a specific feature of the library. We should have sections in `device.rst` explain and providing code examples for all of these features.

