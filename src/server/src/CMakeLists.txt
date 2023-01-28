cmake_minimum_required(VERSION 3.10)

if (MSVC)
cmake_policy(SET CMP0091 NEW)
endif (MSVC)

# set the project name
project(STIServer VERSION 3.0.0 DESCRIPTION "STI Server" LANGUAGES CXX)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif (MSVC)

set(STI_BASE_DIR ../../..)

#FIND_PACKAGE( Boost 1.64 COMPONENTS program_options REQUIRED )

link_directories(${STI_BASE_DIR}/lib)

#add_definitions(-D_ITERATOR_DEBUG_LEVEL=0) 

# add the executable
add_executable(STIServer 
    main.cpp 
    LegacyExperimentXMLBuilder.cpp
    LegacyShotRepository.cpp
    LegacySequenceXMLBuilder.cpp
    ServerDevice.cpp
    tinyxml/tinyxml2.cpp
)
set_property(TARGET STIServer 
    PROPERTY POSITION_INDEPENDENT_CODE ON
    #MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    #MSVC_RUNTIME_LIBRARY "MultiThreaded"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )

target_include_directories(STIServer PRIVATE 
    ${WORKING_DIRECTORY}
    tinyxml
#    ${Boost_INCLUDE_DIR}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/src/network/src
)

# target_link_directories(STIServer PUBLIC 
#     "C:/Users/Jason/Code/dev/sti3/src/device/src/build/install/x64-Debug/lib"
#     "C:/Users/Jason/Code/dev/sti3/src/network/src/build/install/x64-Debug/lib"
# )

target_link_libraries(STIServer 
stidevice 
stinetwork)

install(TARGETS STIServer
    #DESTINATION ${STI_BASE_DIR}/lib
    DESTINATION ../../src/server/bin   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)