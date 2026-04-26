cmake_minimum_required (VERSION 3.20)

# project ("stidocs")
project(stidocs VERSION 3.0.1 DESCRIPTION "STI library docs")

# Add the cmake folder so the FindSphinx module is found
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
set(STI_DIR "${PROJECT_SOURCE_DIR}/..")
#add_subdirectory ("test")




find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

set(STI_HEADER_DIR "${PROJECT_SOURCE_DIR}/../include")
set(STI_DEVICE_SRC_DIR "${PROJECT_SOURCE_DIR}/../src/device/src")
file(GLOB_RECURSE DOXYGEN_INPUT_FILES CONFIGURE_DEPENDS
	${STI_HEADER_DIR}/*.h
	${STI_HEADER_DIR}/*.hpp
	${STI_DEVICE_SRC_DIR}/*.h
	${STI_DEVICE_SRC_DIR}/*.cpp
)

set(DOXYGEN_INPUT_DIR ${STI_DIR}/include)
set(DOXYGEN_DEVICE_SRC_DIR ${STI_DIR}/src/device/src)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/sti3docs/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

set(DOXYGEN_QUIET YES)



#Replace variables inside @@ with the current values
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) #Doxygen won't create this for us

# Only regenerate Doxygen when the Doxyfile or public headers change
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${DOXYGEN_INPUT_FILES}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				   MAIN_DEPENDENCY ${DOXYFILE_IN}
                   COMMENT "Generating doxygen docs")

# Nice named target so we can run the job easily
add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})


set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_SRC ${SPHINX_SOURCE}/src)
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sti3docs/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

set(SPHINX_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/index.rst
	${SPHINX_SRC}/api.rst 
	${SPHINX_SRC}/device.rst
	${SPHINX_SRC}/examples.rst
	${SPHINX_SRC}/interface.rst
	${SPHINX_SRC}/network.rst
	${SPHINX_SRC}/readme.rst
	${SPHINX_SRC}/setuptools.rst
	${SPHINX_SRC}/stipy.rst
	${SPHINX_SRC}/techstack.rst
)

# Only regenerate Sphinx when:
#  - Doxygen has rerun 
#  - Our doc files have been updated
#  - The Sphinx config has been updated
add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
	COMMAND 
		${SPHINX_EXECUTABLE} -b html
		# Tell Breathe where to find the Doxygen output
		-Dbreathe_projects.STI=${DOXYGEN_OUTPUT_DIR}/xml
		${SPHINX_SOURCE} ${SPHINX_BUILD}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS 
		# Other docs files you want to track should go here (or in some variable)
		${SPHINX_FILES}
		${DOXYGEN_INDEX_FILE}
	MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
	COMMENT "Generating documentation with Sphinx")

# Nice named target so we can run the job easily
add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE} ${SPHINX_FILES})



# Zip docs

# Define the directories to include in the zip file
set(files_to_zip sti3docs)

# Define the output zip file
set(zip_file_name "${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz")
message(STATUS "Zip name: ${zip_file_name}")

add_custom_command(OUTPUT ${zip_file_name}
	COMMAND tar "cvfz" "${zip_file_name}" ${files_to_zip}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS 
		${SPHINX_FILES}
		${DOXYGEN_INDEX_FILE}
	MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
	COMMENT "Creating ${zip_file_name}")

add_custom_target(Zip ALL DEPENDS ${zip_file_name})


include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD}
	DESTINATION ${CMAKE_INSTALL_DOCDIR})
