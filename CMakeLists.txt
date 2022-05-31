cmake_minimum_required (VERSION 3.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

project ("NoteVis")

# Include sub-projects.
add_subdirectory ("src")

set_property(TARGET NoteVis PROPERTY CXX_STANDARD 17)

target_include_directories(NoteVis PRIVATE "${CMAKE_SOURCE_DIR}/include")