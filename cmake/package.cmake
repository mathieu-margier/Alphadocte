#
# Package instructions for this project
#

include(InstallRequiredSystemLibraries)
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_BINARY_DIR}/packages")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VERSION_MAJOR "${Alphadocte_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Alphadocte_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${Alphadocte_VERSION_PATCH}")
set(CPACK_PACKAGE_CHECKSUM SHA256)
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CLI program for playing and solving Wordle/Motus-like games, in French.")

if (ALPHADOCTE_OS_LINUX)
  set(CPACK_GENERATOR TGZ)
elseif (ALPHADOCTE_OS_WINDOWS)
  set(CPACK_GENERATOR NSIS64)
endif()

# Debian specific packager options
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Mathieu Margier")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)

include(CPack)
