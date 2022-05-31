cmake_minimum_required (VERSION 3.8)

if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(ARCH_PREFIX "x64")
else ()
  set(ARCH_PREFIX "x86")
endif ()

if (${DEBUG} MATCHES 1)
  message("NoteVis Build Type: Debug")
else ()
  message("NoteVis Build Type: Release")
endif ()

if (${WINDOWS} MATCHES 1)
  message("NoteVis OS: Windows")
else ()
  message("NoteVis OS: Linux")
endif ()

message("NoteVis Arch: ${ARCH_PREFIX}")


# TODO: This way of getting files is not ideal
file(GLOB_RECURSE ALL_SRC_FILES CONFIGURE_DEPENDS "*.h" "*.cpp" "*.hpp" "*.c" "*.cc")
add_executable (NoteVis ${ALL_SRC_FILES})

if (${NV_WINDOWS} MATCHES 1)
  target_link_libraries(NoteVis "${CMAKE_SOURCE_DIR}/lib/${ARCH_PREFIX}/windows/SDL2.lib")
else ()
  find_package(SDL2 REQUIRED)
  target_link_libraries(NoteVis ${SDL2_LIBRARIES})

  find_package(OpenGL REQUIRED)

  include_directories(${OPENGL_INCLUDE_DIRS})
  target_link_libraries(NoteVis ${OPENGL_LIBRARIES})
endif ()

target_link_libraries(NoteVis "${CMAKE_SOURCE_DIR}/lib/${ARCH_PREFIX}/zlibstatic.lib")
  
if (${NV_DEBUG} MATCHES 1)
  target_link_libraries(NoteVis "${CMAKE_SOURCE_DIR}/lib/${ARCH_PREFIX}/fmtd.lib")
else ()
  target_link_libraries(NoteVis "${CMAKE_SOURCE_DIR}/lib/${ARCH_PREFIX}/fmt.lib")
endif ()

if (${NV_WINDOWS} MATCHES 1)
  target_link_libraries(NoteVis "opengl32.lib")
  target_link_libraries(NoteVis "winmm.lib")

  # Move SDL2 DLL over
  configure_file("${CMAKE_SOURCE_DIR}/lib/${ARCH_PREFIX}/windows/SDL2.dll" "${CMAKE_CURRENT_BINARY_DIR}/SDL2.dll" COPYONLY)
endif ()