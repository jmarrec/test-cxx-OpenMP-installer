cmake_minimum_required(VERSION 3.15.0)
# cmake_policy(SET CMP0048 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Do not enable compiler specific extensions, for eg on GCC use -std=c++1z (=c++17) and not -std=gnu++17
set(CMAKE_CXX_EXTENSIONS OFF)

# Use ccache is available, has to be before "project()"
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  # Support Unix Makefiles and Ninja
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

project(testOMP VERSION 0.0.1)

# The PROJECT_SOURCE_DIR here is root of this repo, which we will use instead of using CMAKE_SOURCE_DIR
set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
# We will use BINARY_DIR instead of CMAKE_BINARY_DIR
set(PROJECT_BINARY_DIR "${CMAKE_BINARY_DIR}")

#MESSAGE(FATAL_ERROR "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}, BINARY_DIR=${BINARY_DIR}, PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}, CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}, CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

#if(POLICY CMP0020)
#cmake_policy(SET CMP0020 NEW)
#endif()
#if(POLICY CMP0022)
#cmake_policy(SET CMP0022 NEW)
#endif()
#if(POLICY CMP0042)
#cmake_policy(SET CMP0042 NEW)
#endif()
#if(POLICY CMP0054)
#cmake_policy(SET CMP0054 NEW)
#endif()
#if(POLICY CMP0077)
#cmake_policy(SET CMP0077 NEW)
#endif()

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

###############################################################################
#                                  N I N J A                                  #
###############################################################################
# Ninja support: has to be atop for it to take effect before anything else is done
# Add Color Output if Using Ninja
macro(AddCXXFlagIfSupported flag test)
  check_cxx_compiler_flag(${flag} ${test})
  if(${${test}})
    message("adding ${flag}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
  endif()
endmacro()

if("Ninja" STREQUAL ${CMAKE_GENERATOR})
  # Clang
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    addcxxflagifsupported(-fcolor-diagnostics
                          COMPILER_SUPPORTS_fcolor-diagnostics)
  endif()

  # g++
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # For some reason it doesn't say its supported, but it works...
    # AddCXXFlagIfSupported(-fdiagnostics-color COMPILER_SUPPORTS_fdiagnostics-color)
    message(STATUS "Ninja: Forcing -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  endif()
endif()

# Xcode/Ninja generators undefined MAKE
if(CMAKE_GENERATOR MATCHES "Make")
  set(MAKE "$(MAKE)")
else()
  set(MAKE make)
endif()

###############################################################################
#                         C M A K E    C O N T R O L                          #
###############################################################################

# High level project configuration
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Products")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Products")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Products")

set(LIBRARY_SEARCH_DIRECTORY
    "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Release"
    "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Debug")

###############################################################################
#          C O M P I L E R    A N D    S Y S T E M    O P T I O N S           #
###############################################################################

# Compiler and system specific options
if(UNIX)

  #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC -fno-strict-aliasing")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fno-strict-aliasing")

  # all warnings
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -fno-strict-aliasing -Winvalid-pch -Wnon-virtual-dtor")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS}  -fPIC -fno-strict-aliasing -Winvalid-pch")
  # Treat all warnings as errors, extra errors, and be pedantic
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors -pedantic")
  if(APPLE)
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual -ftemplate-depth=1024")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual")
    # DLM: had to add this due to issues with boost optional and gcc, may be resolved if we move to std::optional
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-maybe-uninitialized")
  endif()

  # Note: CMAKE_CXX_STANDARD set to 20 should already take care of adding -std=c++20 or equivalent
  if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

###############################################################################
#                              E X E C U T A B L E                            #
###############################################################################

set(target_name "test")
find_package(Threads REQUIRED)
find_package(
  OpenMP
  COMPONENTS CXX
  REQUIRED)
add_executable(
  ${target_name}
  # WIN32
  MACOSX_BUNDLE main.cpp)
target_link_libraries(test INTERFACE Threads::Threads)
target_link_libraries(test PRIVATE OpenMP::OpenMP_CXX)

###############################################################################
#                           I N S T A L L A T I O N                           #
###############################################################################

set(DESTDIR "test-install")

set(CPACK_IFW_TARGET_DIRECTORY "${PROJECT_BINARY_DIR}/${DESTDIR}/")

# Default the Binary generators: problem is that none of the CPACK_BINARY_<GenName> will show up in CMakeCache,
# which makes it less clear what will happen in terms of package generation, but it's a dummy example so I don't care!
set(CPACK_GENERATOR "IFW;")

# Turn off source generators
# Need a list, which can't be empty, but not have sensible defined value. So a list of two empty element works as
# a workaround
set(CPACK_SOURCE_GENERATOR ";")

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_RPATH "$ORIGIN")

install(
  TARGETS ${target_name}
  DESTINATION .
  COMPONENT MyTest)

#message(AUTHOR_WARNING "OpenMP_CXX_LIBRARIES=${OpenMP_CXX_LIBRARIES}")

#message(AUTHOR_WARNING "OpenMP_CXX_LIB_NAMES=${OpenMP_CXX_LIB_NAMES}")
#message(AUTHOR_WARNING "OpenMP_gomp_LIBRARY=${OpenMP_gomp_LIBRARY}")

# Does not work
# install(PROGRAMS $<TARGET_FILE:OpenMP::OpenMP_CXX> DESTINATION "./" COMPONENT libomp )

foreach(_lib_name ${OpenMP_CXX_LIB_NAMES})
  message(AUTHOR_WARNING "_lib_name=${_lib_name}")
  set(_lib_path ${OpenMP_${_lib_name}_LIBRARY})
  get_filename_component(_lib_base_name ${_lib_path} NAME)

  message("_lib_path=${_lib_path}")
  get_filename_component(_lib_location_resolved "${_lib_path}" REALPATH)
  message("_lib_location_resolved=${_lib_location_resolved}")
  get_filename_component(_lib_location_resolved_base_name
                         ${_lib_location_resolved} NAME)

  if(NOT _lib_base_name STREQUAL _lib_location_resolved_base_name)
    add_custom_command(
      TARGET ${target_name}
      POST_BUILD
      COMMAND
        ${CMAKE_COMMAND} -E create_symlink ${_lib_location_resolved_base_name}
        $<TARGET_FILE_DIR:${target_name}>/${_lib_base_name})
    install(
      PROGRAMS "$<TARGET_FILE_DIR:${target_name}>/${_lib_base_name}"
      DESTINATION "./"
      COMPONENT libomp)
  endif()
  add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_lib_location_resolved}
            $<TARGET_FILE_DIR:${target_name}>/
    # COMMAND chrpath -r "$ORIGIN/." $<TARGET_FILE_DIR:${target_name}>/QtWebEngineProcess
    VERBATIM)
  install(
    PROGRAMS "${_lib_location_resolved}"
    DESTINATION "./"
    COMPONENT libomp)
endforeach()

include(CPack)
include(CPackIFW)

cpack_add_component(
  MyTest
  DISPLAY_NAME "Entire program"
  DESCRIPTION "This is a description")

cpack_add_component(
  libomp
  DISPLAY_NAME "the libomp library"
  DESCRIPTION "This is a description")
