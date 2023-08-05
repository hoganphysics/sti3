.. include global.rst

=====
Setup
=====

Setting up STI for the first time.

Windows
-------

Build STI3 in windows

* Install `openSSL <https://www.openssl.org/>`_

Prebuilt Windows binaries: `<https://slproweb.com/products/Win32OpenSSL.html>`_  (Win64, do not use Light)


* Install `pybind11 <https://pybind11.readthedocs.io/en/stable/installing.html>`_

.. code-block:: bash

        pip install pybind11


* Install `omniORB <https://omniorb.sourceforge.io/>`_
  
.. Note::
    Building omniORB from source may be required if the available Windows binaries are not compatible with the build tools (e.g., Visual Studio version).
    To build omniORB, first install: cygwin, python, openssl. Following the instructions in README.win32.txt, select and then modify the 
    appropriate .mk file in in mk/platforms.
    When installing cygwin, make sure to install GNU make.
    To build, run 'make export' using the x64 Native cmd prompt for Visual Studio.
    Make sure NOT to install openssl in the 'Program Files' directory, since the space in the name breaks the build!

* Install `Boost <https://www.boost.org/users/download/>`_ (Header only libraries)

.. Note::
    STI requires only `Boost Graph Library <https://www.boost.org/doc/libs/1_82_0/libs/graph/doc/index.html>`_ (BGL). 
    BGL is a header-only library and does not need to be built.

* Define the following environment variables:

.. code-block:: bash

        OPENSSL_ROOT_DIR
        OMNIORB_ROOT_DIR
        BOOST_ROOT

These should be defined as their respective directories of these on the local computer.
OPENSSL_ROOT_DIR should point to the root directory of openSSL.
OMNIORB_ROOT_DIR should point to the root directory of omniORB.
BOOST_ROOT should point to the root directory of boost.

.. Note:: 
    These environment variables are also needed for building the python wheel.

* Build STI3 in Visual Studio
  
  * Open root directory 'sti3'; CMake config should automatically run to prepare the build.
  * Select Debug or Release build target
  * Build the project.  This will build the stidevice and stinetwork libraries.
  * The compiled libraries for linking will be in sti3\\lib and the dlls will be in sti3\\bin. Debug libraries have a '_d' suffix.

.. warning:: 
    For a CMake build, Visual Studio needs access to executables in the System32 directory. 
    Without access, the CMake config step may give an error claiming to not find cmd.exe, for example.
    One way to fix this is to append %SystemRoot%\\System32 to the Path environment variable.

Build STI3 Python in Windows
----------------------------

* Setup python virtual environment for build (optional)
* Install required python packages for build using requirements.txt in the root directory

.. code-block:: bash

    pip install -r requirements.txt


* Build wheel

.. code-block:: bash

    python -m build --wheel

* The created whl file will be in the sti3/dist directory.

