.. include global.rst

=====
Setup
=====

Setting up STI for the first time.

Build using Docker
------------------

The easiest way to build the STI3 library is using Docker. The Dockerfile for
building the library is located in the root directory: `sti3/Dockerfile`.
From the root sti3 directory, run the following:

.. code-block:: bash
    
    docker build -t sti3:v1 .

The image name and version are optional.  In addition to installing all build dependencies,
this Dockerfile will download and build omniORB before building STI3.


Build from source (Linux)
-------------------------

* Install `openSSL` and `curl`

.. code-block:: bash

    apt install libssl-dev openssl libcurl4-openssl-dev


* Install `pybind11`

.. code-block:: bash

    pip3 install pybind11


* Install `omniORB` from source

.. code-block:: bash

    # Download latest source zip
    wget https://sourceforge.net/projects/omniorb/files/latest/download

    # unzip; change to $OMNIORB_TOP (root directory of omniORB)
    cd $OMNIORB_TOP
    mkdir build
    cd build

    ../configure --with-openssl
    make
    make install

* Install `STI3` from source

.. code-block:: bash

    # Download latest STI3 source zip

    # Change to build directory
    cd sti3/build

    # Configure.  For debug builds, include option -DCMAKE_BUILD_TYPE=Debug
    cmake ..

    # Build
    cmake --build . --parallel 4

    # Install
    make DESTDIR=/sti3 install



Build from source (Windows)
---------------------------


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

Build STI3 Python (STIPy)
-------------------------

The build system for STIPy uses `setuptools` to create the python package.  The `setup.py` file in the 
root directory configures the build. The python build will configure the package and call `cmake` to compile
the c++ code for the core STI shared libraries. The output of thee build is a platform-specific python wheel 
(whl file) which contains the complied STI binaries. The following build instructions are platform independent.

* Setup python virtual environment for the build (optional)
* Install the required python packages for the build using `requirements.txt` in the root directory

.. code-block:: bash

    pip install -r requirements.txt


* Build the wheel

.. code-block:: bash

    python -m build --wheel

* The created whl file will be in the sti3/dist directory.

* (Optional) Install STIPy wheel directly (in the desired virtual environment)

.. code-block:: bash

    pip install -I ./sti3/dist/<whl filename>

