import sys
import os
import glob
import shutil


try:
    from skbuild import setup
except ImportError:
    print(
        "Please update pip, you need pip 10 or greater,\n"
        " or you need to install the PEP 518 requirements in pyproject.toml yourself",
        file=sys.stderr,
    )
    raise

#Force sci-kit to use RelWithDebInfo build instead of Release
#Release fails to link on the first build (for some reason) causing a crash.
sys.argv.append('--build-type=RelWithDebInfo')
#print("command line args: " + str(sys.argv))

from setuptools import find_packages
#from setuptools import find_namespace_packages

OMNIORBBASE="C:/Users/Jason/Code/lib/omniORB-4.2.4.x64"

def copyOmniORBlinux():
    #glob.glob("/usr/local/lib/libomniORB*.so*")
    slibPath = "/usr/local/lib/"
    omniorbTargetLibs = ["libomniORB*.so*", "libomnithread.so*", "libomniDynamic*.so*"]
    copyOmniORB(slibPath, omniorbTargetLibs)

def copyOmniORBwindows():
    slibPath = OMNIORBBASE + "/bin/x86_win32/"
    omniorbTargetLibs = ["omniORB*_rt.dll", "omnithread*_rt.dll", "omniDynamic*_rt.dll"]
    copyOmniORB(slibPath, omniorbTargetLibs)

def copyOmniORB(slibPath, omniorbTargetLibs):

    print("Moving OmniORB shared library to stipy/bin for python wheel...")

    #slibPath = "/bin/x86_win32/"
    #omniorbTargetLibs = ["omniORB*_rt.dll", "omnithread*_rt.dll", "omniDynamic*_rt.dll"]

    #Find target shared libs
    sharedlibs=[]
    for x in omniorbTargetLibs:
      matches = glob.glob(slibPath + x)
      if matches:
        sharedlibs.extend(matches)
        #sharedlibs.append(matches[0])

    #Destination directory
    stipybinDirectory = os.getcwd() + "/src/stipy/bin"

    #Copy
    for slib in sharedlibs:
      print("Exists?" + str(slib) + " " + str(os.path.lexists(stipybinDirectory + "/" + os.path.basename(slib))))
      if not os.path.lexists(stipybinDirectory + "/" + os.path.basename(slib)):
        print("Copying " + slib + " --> " + stipybinDirectory)
        shutil.copy2(slib, stipybinDirectory, follow_symlinks=False)

    #Check
    libsFound = 0
    for x in omniorbTargetLibs:
      matches = glob.glob(stipybinDirectory + "/" + x)
      if matches:
        libsFound += 1

    if libsFound == len(omniorbTargetLibs):
        print("All omniORB libs copied!")
    else:
        print("Failed to find all omniORB libs")
        print("Tried to find: " + str(omniorbTargetLibs))


#Put required omniORB shared libs in stipy/bin for distribution with wheel
#copyOmniORB(OMNIORBBASE)
if sys.platform == 'linux':
    copyOmniORBlinux()
else:
    copyOmniORBwindows()


setup(
    name="stipy",
    version="3.0.1",
    description="STIPy",
    author="Jason Hogan",
    license="MIT",
    #packages=find_namespace_packages(where="src"),
    #packages=find_packages(where="src"),
    #package_dir={"": "src"},
    packages=['stipy','stipy.stidevicepy','stipy.stipybase','stipy.stipybase.python','stipy.bin', 'stipy.python'],
    package_dir={"": "src"},
    cmake_install_dir="",
	#cmake_args=['-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DOMNIORB_PATH=C:/Users/Jason/Code/lib/omniORB-4.2.4.x64'],
  	cmake_args=['-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DOMNIORB_PATH=' + OMNIORBBASE, '-DCMAKE_BUILD_PARALLEL_LEVEL=4', '-DCMAKE_JOB_POOL_COMPILE:STRING=compile', '-DCMAKE_JOB_POOL_LINK:STRING=link', '-DCMAKE_JOB_POOLS:STRING=compile=5;link=2'],
    include_package_data=True,
    exclude_package_data={"": ["*.lib","bin/*"]},
    #data_files=[('bin', ['bin/stidevice.dll','bin/stinetwork.dll'])],
    #package_data={"stipy.bin": ["*.dll","*.pyd"]},
    #package_data={"stipy.bin": ["omniORB424_vc16_rt.dll"]},
    package_data={"stipy.bin": ["*.dll", "*.so*"]},
    extras_require={}, #{"test": ["pytest"]},
    python_requires=">=3.6",
)