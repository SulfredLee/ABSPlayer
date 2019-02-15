set(targetName "ABSPlayer")
get_filename_component(folderName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" folderName ${folderName})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-unknown-pragmas")

file(GLOB ${folderName}_inc
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Backend/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerMsg/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Util/*.h")
file(GLOB ${folderName}_src
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Backend/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlayerMsg/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Util/*.cpp")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Util
  ${CMAKE_CURRENT_SOURCE_DIR}/PlayerMsg
  ${CMAKE_CURRENT_SOURCE_DIR}/Backend
  ${CMAKE_CURRENT_SOURCE_DIR}/libdash/include
  ${libxml2_INCLUDE_DIR}
  ${libcurl_INCLUDE_DIR})

add_executable(${targetName} ${${folderName}_src})

target_link_libraries(${targetName}
  ${CMAKE_THREAD_LIBS_INIT}
  libdash
  ${libxml2_LIBRARY_DIR}/libxml2.so.2.9.8
  ${libcurl_LIBRARY_DIR}/libcurl.so.4.5.0)

add_subdirectory(libdash)

# Creates a folder "executables" and adds target
# project (*.vcproj) under it
set_property(TARGET ${targetName} PROPERTY FOLDER "executables")

# Adds logic to INSTALL.vcproj to copy *.exe to destination directory
install (TARGETS ${targetName} DESTINATION bin)
