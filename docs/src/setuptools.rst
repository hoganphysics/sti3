===============
Installing STI3
===============

The recommended way to install STI3 is the ``stipy`` conda package.  The
package includes the Python bindings, the C++ STI libraries, public headers,
and the installed example code for both Python and C++.

Install from Anaconda Cloud
---------------------------

Create an environment for using STI3 and install ``stipy`` from the Hogan Lab
Anaconda channel:

.. code-block:: bash

   conda create -n sti3 python=3.13
   conda activate sti3
   conda install -c conda-forge hoganlab::stipy

For an existing environment:

.. code-block:: bash

   conda activate sti3
   conda install -c conda-forge hoganlab::stipy

After installation, verify the Python package:

.. code-block:: bash

   python -c "import stipy; import stipy.stidevicepy; print(stipy.__file__)"

What gets installed
-------------------

The ``stipy`` package installs the full STI runtime needed by device applications:

* Python modules: ``stipy`` and ``stipy.stidevicepy``.
* C++ headers under the conda prefix include directory, for example
  ``$CONDA_PREFIX/include/sti``.
* C++ libraries and runtime binaries under the conda prefix library and binary
  directories.
* Example projects showing Python and C++ device drivers.

Use the active conda environment as the install prefix.  On Linux and macOS
this is ``$CONDA_PREFIX``.  On Windows this is ``%CONDA_PREFIX%`` in Command
Prompt or ``$env:CONDA_PREFIX`` in PowerShell.

Python device project
---------------------

A Python device can import ``stipy`` directly from the activated environment:

.. code-block:: py

   import stipy
   import stipy.stidevicepy as stidevicepy

   class SimpleDevice(stidevicepy.LocalDevice):
       def __init__(self, config):
           stidevicepy.LocalDevice.__init__(self, config)
           self.addOutputChannel(0, stipy.MixedValueType.Double, "coil current")

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

See ``examples/python/simpleDevice`` and the feature-specific examples under
``examples/python`` for complete runnable files.

C++ device project with CMake
-----------------------------

For a small out-of-tree C++ device project, point CMake at the active conda
environment and link against the installed STI libraries.  The exact library
target names can vary by package platform, so the most portable starting point
is to add the conda include and library directories explicitly:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.20)
   project(my_sti_device LANGUAGES CXX)

   set(CMAKE_CXX_STANDARD 20)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)

   if(NOT DEFINED ENV{CONDA_PREFIX})
       message(FATAL_ERROR "Activate the conda environment that contains stipy.")
   endif()

   set(STI_PREFIX "$ENV{CONDA_PREFIX}")

   add_executable(my_sti_device main.cpp)
   target_include_directories(my_sti_device PRIVATE "${STI_PREFIX}/include")
   target_link_directories(my_sti_device PRIVATE "${STI_PREFIX}/lib")
   target_link_libraries(my_sti_device PRIVATE stidevice stinetwork)

On Windows with Visual Studio, use ``%CONDA_PREFIX%\include`` as an additional
include directory, ``%CONDA_PREFIX%\Library\lib`` or ``%CONDA_PREFIX%\lib`` as
an additional library directory depending on the package layout, and add the
installed STI libraries to the linker input.  Put the conda binary directories
on ``PATH`` when running the executable:

.. code-block:: bat

   conda activate sti3
   set PATH=%CONDA_PREFIX%\Library\bin;%CONDA_PREFIX%\bin;%PATH%

The installed C++ examples contain complete ``CMakeLists.txt`` files.  They are
the best template when starting a new device driver.

Build a conda package from source
---------------------------------

Most users should install from Anaconda Cloud.  Build from source only when
developing STI3 itself or testing package changes.

Create the build environment:

.. code-block:: bash

   conda env create -f environment-dev.yml
   conda activate sti3-build

Update an existing environment after ``environment-dev.yml`` changes:

.. code-block:: bash

   conda env update --name sti3-build --file environment-dev.yml --prune

Build from the repository root:

.. code-block:: bash

   conda activate sti3-build
   conda build -c conda-forge .

Install the local build into a target environment:

.. code-block:: bash

   conda activate sti3
   conda install --use-local stipy

If conda does not find the local package, add the local build channel first:

.. code-block:: bash

   conda config --add channels file://$(conda info --base)/envs/sti3-build/conda-bld

Legacy source build notes
-------------------------

The previous Docker, manual omniORB, and Python wheel instructions have been
kept for reference in ``docs/src/setuptools_legacy.rst``.  They are no longer
the recommended installation path.
