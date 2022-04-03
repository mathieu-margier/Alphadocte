cmake_minimum_required(VERSION 3.14...3.23)

# set the project name
project(Alphadocte VERSION 1.0.0 LANGUAGES CXX)

# Define default variable values
include("cmake/defaults.cmake")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

message(STATUS "Target system name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Host system name: ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "Compiler id: ${CMAKE_CXX_COMPILER_ID}")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(ALPHADOCTE_OS_LINUX 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(ALPHADOCTE_OS_WINDOWS 1)
else()
  message(FATAL_ERROR "Unsupported build operating system: ${CMAKE_SYSTEM_NAME}")
endif()

# Add extra warnings
if (COMPILE_WITH_EXTRA_WARNING)
  # extra warning and all warnings as errors
  # cannot add spaces in generator expressions
  add_compile_options(
    # Visual compilers
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
      $<$<CXX_COMPILER_ID:MSVC>:/WX>
    # Others
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-pedantic>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
  )
  
  if(MSVC)
    message(WARNING "Compilation with extra warning has not been tested with MSVC, it will most likely fail.")
  endif()
endif()

# External dependencies (include them to make found targets visible in the main scope and sub scopes)
include(3rd_party/CMakeLists.txt)

# Core library
add_subdirectory(src)

# Apps (CLI player and solver)
add_subdirectory(apps)

# Tests (if enabled and this project is toplevel)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Install targets
include("cmake/install.cmake")

# Package application
include("cmake/package.cmake")
