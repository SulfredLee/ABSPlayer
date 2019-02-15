set(targetName "libdash")
get_filename_component(folderName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" folderName ${folderName})

file(GLOB ${folderName}_inc
  "${CMAKE_CURRENT_SOURCE_DIR}/source/helpers/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/manager/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/metrics/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/mpd/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/network/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/portable/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/xml/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB ${folderName}_src
  "${CMAKE_CURRENT_SOURCE_DIR}/source/helpers/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/manager/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/metrics/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/mpd/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/network/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/portable/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/xml/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${libxml2_INCLUDE_DIR}
  ${libcurl_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

# state that this project is a library
add_library(${targetName} ${${folderName}_src}) # static library
# add_library(${targetName} SHARED ${${folderName}_src}) # dynamic library
target_link_libraries(${targetName}
  ${libxml2_LIBRARY_DIR}/libxml2.so.2.9.8
  ${libcurl_LIBRARY_DIR}/libcurl.so.4.5.0)

# Creates a folder "libraries" and adds target project (*.vcproj) under it
set_property(TARGET ${targetName} PROPERTY FOLDER "libraries")

# Adds logic to INSTALL.vcproj to copy *.a to destination directory
install (TARGETS ${targetName} DESTINATION lib)
install (FILES ${${folderName}_inc} DESTINATION include)
