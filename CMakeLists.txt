cmake_minimum_required (VERSION 2.8.12)

# Maps to a solution filed (*.sln). The solution will
# have all targets (exe, lib, dll) as projects (.vcproj)
project(ABSPlayer)

# The version number.
set (Project_VERSION_MAJOR 1)
set (Project_VERSION_MINOR 0)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
set(CMAKE_CXX_FLAGS "-Wall -fPIC -std=c++11 -g")
message(STATUS "Root - cxx Flags: " ${CMAKE_CXX_FLAGS})

if (BUILD_TOOL MATCHES "STB")
  set(tools /usr/local/stbgcc-4.8-1.6)
  set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gcc)
  set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-g++)
endif ()

# Handle Preprocess Flags
if (UNIX)
  add_definitions(-DUNIX)
else ()
  add_definitions(-DWINDOWS -DWIN32 "/EHsc")
endif ()

# Handle linux libraries
if (UNIX)
  find_package(
    Threads) # include this package to fix problem 001
else ()
endif ()
message(STATUS "Info - CMAKE_THREAD_LIBS_INIT: ${CMAKE_THREAD_LIBS_INIT}")

# Handle libxml2 libaries
message(STATUS "Info - preparing for libxml2 -------------------------")
if (UNIX)
  if (BUILD_TOOL MATCHES "STB")
    set(libxml2_ROOT ${STB_STAGING_DIR})
  else ()
    set(libxml2_ROOT "/usr/local/libxml2/2.9.8/")
  endif ()
  set(libxml2_INCLUDE_DIR ${libxml2_ROOT}include/libxml2)
  set(libxml2_LIBRARY_DIR ${libxml2_ROOT}lib)
else ()
endif ()
if (NOT EXISTS "${libxml2_INCLUDE_DIR}/libxml/uri.h")
  message(FATAL_ERROR "Error - ${libxml2_INCLUDE_DIR}/uri.h not found")
  return()
endif()
if (NOT EXISTS "${libxml2_LIBRARY_DIR}/libxml2.a")
  message(FATAL_ERROR "Error - ${libxml2_LIBRARY_DIR}/libxml2.a not found")
  return()
endif()
message(STATUS "Info - libxml2 include path: ${libxml2_INCLUDE_DIR}")
message(STATUS "Info - libxml2 library path: ${libxml2_LIBRARY_DIR}")

# Handle curl libaries
message(STATUS "Info - preparing for libcurl -------------------------")
if (UNIX)
  if (BUILD_TOOL MATCHES "STB")
    set(libcurl_ROOT ${STB_STAGING_DIR})
  else ()
    set(libcurl_ROOT "/usr/local/libcurl/7.63.0/")
  endif ()
  set(libcurl_INCLUDE_DIR ${libcurl_ROOT}include)
  set(libcurl_LIBRARY_DIR ${libcurl_ROOT}lib)
else ()
endif ()
if (NOT EXISTS "${libcurl_INCLUDE_DIR}/curl/curl.h")
  message(FATAL_ERROR "Error - ${libcurl_INCLUDE_DIR}/curl/curl.h not found")
  return()
endif()
if (NOT EXISTS "${libcurl_LIBRARY_DIR}/libcurl.a")
  message(FATAL_ERROR "Error - ${libcurl_LIBRARY_DIR}/libcurl.a not found")
  return()
endif()
message(STATUS "Info - libcurl include path: ${libcurl_INCLUDE_DIR}")
message(STATUS "Info - libcurl library path: ${libcurl_LIBRARY_DIR}")

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(ABSPlayer)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${Project_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${Project_VERSION_MINOR}")
include (CPack)

# Problem 001
#        /usr/bin/ld: ... undefined reference to symbol 'pthread_rwlock_wrlock@@GLIBC_2.2.5'
