# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\OpenGL_0_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\OpenGL_0_autogen.dir\\ParseCache.txt"
  "OpenGL_0_autogen"
  )
endif()
