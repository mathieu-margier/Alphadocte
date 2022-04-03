#
# Install instructions for this project
#

# We do not export a cmake's package file or includes, since this a private library

# Find word lists
file(GLOB WORDLISTS CONFIGURE_DEPENDS data/*_wordlist.txt)

# todo if build static, do not install lib

if (ALPHADOCTE_OS_LINUX)
  include(GNUInstallDirs)

  # Install (read-only) data, ie wordlists
  install(
    FILES ${WORDLISTS}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/alphadocte)
    
  # Install targets (library and executables)
  install(TARGETS alphadocte alphadocte-player alphadocte-solver)
elseif(ALPHADOCTE_OS_WINDOWS)
  # Install (read-only) data, ie wordlists
  install(
    FILES ${WORDLISTS}
    DESTINATION data)

  # Install targets (library and executables)
  install(TARGETS alphadocte alphadocte-player alphadocte-solver RUNTIME DESTINATION ".")
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
  
  if (MINGW)
    if (NOT CMAKE_CROSSCOMPILING)
      # Workaround to install MinGW system libs alongside executables on Windows
      # InstallRequiredSystemLibraries can fetch MSVC runtime, but not MinGW yet.
      message(STATUS "Mingw: manually include system libs for installation.")
      message(WARNING "System libs are not embedded when packing the application, they need to be added manually.\nOne can copy the system libs within the install tree and put them in the packages.")

      # Pass CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION to CMake install script
      install(CODE "set(DEP_INSTALL_DIR \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION}\")")
    
      # TODO 
      # Works for installation, but not packaging since we use file and not install
      # (which is not available inside install code...)
      install(CODE [[
        message(STATUS "DEP_INSTALL_DIR=${DEP_INSTALL_DIR})")
        message(STATUS "Looking for MinGW system libraries in PATH")
        file(GET_RUNTIME_DEPENDENCIES
          # tell cmake to consider those libraries as system libs
          RESOLVED_DEPENDENCIES_VAR deps_resolved
          UNRESOLVED_DEPENDENCIES_VAR deps_unresolved
          LIBRARIES "$<TARGET_FILE:alphadocte>"
          EXECUTABLES "$<TARGET_FILE:alphadocte-player>" "$<TARGET_FILE:alphadocte-solver>"
          DIRECTORIES $ENV{PATH}
          # include MinGW64 system libs
          PRE_INCLUDE_REGEXES [=[^libgcc.*\.dll$]=] [=[^libstdc\+\+-[0-9]+\.dll$]=] [=[libwinpthread-[0-9]+.dll$]=]
          PRE_EXCLUDE_REGEXES "." # exclude all dll by default
          )
        foreach(dep ${deps_resolved})
          file(INSTALL "${dep}" DESTINATION "${DEP_INSTALL_DIR}")
        endforeach()
        foreach(dep ${deps_unresolved})
          message(WARNING "Could not find system library ${dep}. Are MinGW's libraries in PATH?")
        endforeach()
      ]])
    else()
      message(WARNING "Cross-compiling with MinGW: cannot automatically ship mingw64 system libs alongside binaries.\nOne can search libs in the toolchain's bin folder, usually named: libgcc*.dll, libwinpthread-*.dll, libstdc++-*.dll.")
    endif()
  endif()
else()
  message(WARNING "Undefined installation instructions for the target OS")
endif()
