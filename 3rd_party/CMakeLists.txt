# List third party dependencies
include(FetchContent)

# Termcolor - color terminal output on Unix-like OSs and Windows
FetchContent_Declare(
    termcolor
    GIT_REPOSITORY  https://github.com/ikalnytskyi/termcolor.git
    GIT_TAG         b3cb0f365f8435588df7a6b12a82b2ac5fc1fe95 #master
)

# Catch2 - unit testing inspired by PyTest
FetchContent_Declare(
    catch2
    GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
    GIT_TAG         216713a4066b79d9803d374f261ccb30c0fb451f #v2.13.8
)


# boost
set(ALPHADOCTE_BOOST_USE_CONFIG_PACKAGE ON CACHE BOOL "Search for BoostConfig.cmake instead of legacy CMake's find package. Can be turned off if boost has not been installed with CMake integration.")

set(ALPHADOCTE_BOOST_EXTRA_ARGS "")
if (ALPHADOCTE_BOOST_USE_CONFIG_PACKAGE)
  list(APPEND ALPHADOCTE_BOOST_EXTRA_ARGS CONFIG)
endif()

find_package(Boost REQUIRED ${ALPHADOCTE_BOOST_EXTRA_ARGS})

# Import declared content
message(STATUS "Downloading dependencies, if not yet done...")
FetchContent_MakeAvailable(catch2)
message(STATUS "Downloading dependencies, if not yet done... - DONE")

list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib") # Needed to include Catch macros
include(Catch)

# Dont use FetchContent_MakeAvailable to prevent header only dependencies from being installed
# Use regular FetchContent_Populate instead
if(NOT termcolor_POPULATED)
  message(STATUS "Downloading termcolor...")
  FetchContent_Populate(termcolor)
  message(STATUS "Downloading termcolor... - DONE")

  # Bring the populated content into the build, with exclude_from_all to prevent installation
  add_subdirectory(${termcolor_SOURCE_DIR} ${termcolor_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

