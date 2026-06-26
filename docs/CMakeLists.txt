cmake_minimum_required (VERSION 3.20)

if(NOT DEFINED STI3_VERSION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/StiVersion.cmake")
  sti3_load_version("${CMAKE_CURRENT_SOURCE_DIR}/..")
endif()

# project ("stidocs")
project(stidocs VERSION ${STI3_VERSION} DESCRIPTION "STI library docs")

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

set(STI_DOCS_SITE_ROOT ${CMAKE_CURRENT_BINARY_DIR}/sti3docs)
set(SITE_INDEX_IN ${CMAKE_CURRENT_SOURCE_DIR}/site_index.html.in)
set(SITE_INDEX_FILE ${STI_DOCS_SITE_ROOT}/index.html)



#Replace variables inside @@ with the current values
file(MAKE_DIRECTORY ${STI_DOCS_SITE_ROOT})
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
configure_file(${SITE_INDEX_IN} ${SITE_INDEX_FILE} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) #Doxygen won't create this for us

# Only regenerate Doxygen when the Doxyfile or public headers change
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${DOXYGEN_INPUT_FILES}
                   COMMAND ${CMAKE_COMMAND} -E rm -rf ${DOXYGEN_OUTPUT_DIR}
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${DOXYGEN_OUTPUT_DIR}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				   MAIN_DEPENDENCY ${DOXYFILE_IN}
                   COMMENT "Generating doxygen docs")

# Nice named target so we can run the job easily
add_custom_target(Doxygen DEPENDS ${DOXYGEN_INDEX_FILE})
add_custom_target(SiteIndex DEPENDS ${SITE_INDEX_FILE})


set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_SRC ${SPHINX_SOURCE}/src)
set(SPHINX_BUILD ${STI_DOCS_SITE_ROOT}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)
set(STI3_VERSION_FILE ${STI_DIR}/sti3_version.json)
set(SPHINX_FOOTER_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/_templates/footer.html)

set(SPHINX_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/index.rst
	${STI3_VERSION_FILE}
	${SPHINX_FOOTER_TEMPLATE}
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
	COMMAND ${CMAKE_COMMAND} -E rm -rf ${SPHINX_BUILD}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${SPHINX_BUILD}
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
add_custom_target(Sphinx DEPENDS ${SPHINX_INDEX_FILE} ${SPHINX_FILES})
add_custom_target(DocsSite ALL DEPENDS SiteIndex Sphinx)



# Zip docs

# Define the directories to include in the zip file
set(files_to_zip sti3docs)

# Define the output zip file
set(zip_file_name "${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz")
message(STATUS "Zip name: ${zip_file_name}")

add_custom_command(OUTPUT ${zip_file_name}
	COMMAND tar "czf" "${zip_file_name}" ${files_to_zip}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS 
		${SITE_INDEX_FILE}
		${SPHINX_INDEX_FILE}
		${DOXYGEN_INDEX_FILE}
	MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
	COMMENT "Creating ${zip_file_name}")

add_custom_target(Zip DEPENDS ${zip_file_name})


include(GNUInstallDirs)
install(DIRECTORY ${STI_DOCS_SITE_ROOT}
	DESTINATION ${CMAKE_INSTALL_DOCDIR})
