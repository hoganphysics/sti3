
src/

Contains the SWIG wrapper code that generates the java source of the stidevice lib.
The emitted java source is copied to stilib/src/java.
Also contains c++ wrapper code to make stidevice java friendly.
Builds a native shared lib that is loaded by java System.loadLibrary.


stilib/

Target for the SWIG wrapper library.  Builds a JAR that wraps the STI library.
Loads the native library code with System.loadLibrary on startup.
