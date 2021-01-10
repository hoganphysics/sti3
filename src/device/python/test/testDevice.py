
##Need to run
# export LD_LIBRARY_PATH=.
#From cmd line before package will work.
# LD_LIBRARY_PATH tells python where to look for *.so files.
# Here we just add the local dir temporarily...
#https://stackoverflow.com/questions/1099981/why-cant-python-find-shared-objects-that-are-in-directories-in-sys-path

# from setuptools import setup

# setup(
#     name='libstidevice',
#     version=3,  # specified elsewhere
#     package_dir={'': '.'},
#     package_data={'': ['libstidevice.so']}
# )

from example import *

d=Dog()

print(call_go(d))

dev=DeviceID("Test Dev", "localhost", 22)

print(dev.getName())
print(dev)

dev2=DeviceID("Test Dev", "localhost", 22, "/srv1/")
print(dev2.getTargetServerID())
