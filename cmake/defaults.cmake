#
# Defines default values for CMake variables
#
set(default_build_shared_libs ON)

# Set default build type to release
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Build shared libraries by default
if (NOT DEFINED BUILD_SHARED_LIBS)
  message(STATUS "Setting BUILD_SHARED_LIBS to '${default_build_shared_libs}' as none was specified.")
  set(BUILD_SHARED_LIBS "${default_build_shared_libs}" CACHE BOOL "Build shared libraries." FORCE)
endif()

# Build with extra warnings by default
if (NOT DEFINED COMPILE_WITH_EXTRA_WARNING)
  message(STATUS "Do not compile with extra warning since COMPILE_WITH_EXTRA_WARNING was not defined.")
  SET(COMPILE_WITH_EXTRA_WARNING OFF CACHE BOOL "Add compiler flags to emit more warning messages." FORCE)
endif()
