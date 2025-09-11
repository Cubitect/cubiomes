cmake_minimum_required(VERSION 3.16)
project(seedscan C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Default to Release for speed unless the user overrides
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(seedscan src/main.c)

# --- Cubiomes integration ---
# Expect cubiomes as a submodule at external/cubiomes (adjust if needed)
# add_subdirectory(external/cubiomes)
# target_link_libraries(seedscan PRIVATE cubiomes)
# target_include_directories(seedscan PRIVATE external/cubiomes)
#
# With the current stubs, the file builds without cubiomes so the agent
# can wire those calls in a follow-up commit without breaking the build.
