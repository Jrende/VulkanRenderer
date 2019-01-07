#TODO: Create find_package for glfw, glew, glm
cmake_minimum_required (VERSION 2.6)
project (Vulkan)

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -static-libgcc -static-libstdc++")

MESSAGE("VULKAN_SDK: $ENV{VULKAN_SDK}")
#MESSAGE("Vulkan library: ${VULKAN_LIBRARY}")

SET(GLFW_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
SET(GLFW_BUILD_TESTS OFF CACHE STRING "" FORCE)

add_definitions(-DNDEBUG)

add_subdirectory(lib/glfw-3.2.1)

include_directories (SYSTEM
  lib/glfw/include
  $ENV{VULKAN_SDK}/include
  lib/glm
  lib/include
)

link_directories($ENV{VULKAN_SDK}/lib)

#The order is important!
set(ALL_LIBS
  glfw
  ${GLFW_LIBRARIES}
  Xcursor
  Xinerama
  Xi
  vulkan
)

MESSAGE("Include directories: ${INCLUDE_DIRECTORIES}")

file(GLOB_RECURSE HONDO_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE HONDO_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h")

set (HONDO_INCLUDE_DIRS "")
foreach (_headerFile ${HONDO_HEADERS})
  get_filename_component(_dir ${_headerFile} PATH)
  list (APPEND HONDO_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES HONDO_INCLUDE_DIRS)

include_directories(${HONDO_INCLUDE_DIRS})
add_executable (Vulkan ${HONDO_SOURCES})
target_link_libraries(Vulkan ${ALL_LIBS})
set_property(TARGET Vulkan APPEND PROPERTY COMPILE_FLAGS "-g -Wall -Wextra -Wno-unused-parameter")
