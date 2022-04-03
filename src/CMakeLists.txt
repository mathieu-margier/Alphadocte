# specify src and include folders
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(SRC_INC_DIR "${PROJECT_SOURCE_DIR}/include")
set(GEN_INC_DIR "${PROJECT_BINARY_DIR}/include")
set(INC_DIRS "${SRC_INC_DIR}" "${GEN_INC_DIR}")

# specify which include files will define the project version
configure_file("${SRC_INC_DIR}/Alphadocte/Alphadocte.h.in" "${GEN_INC_DIR}/Alphadocte/Alphadocte.h")

# list header files
set(GEN_INC_FILES
    "${GEN_INC_DIR}/Alphadocte/Alphadocte.h"
)

set(SRC_INC_FILES
    "${SRC_INC_DIR}/Alphadocte/Dictionary.h"
    "${SRC_INC_DIR}/Alphadocte/EntropyMaximizer.h"
    "${SRC_INC_DIR}/Alphadocte/Exceptions.h"
    "${SRC_INC_DIR}/Alphadocte/FixedSizeDictionary.h"
    "${SRC_INC_DIR}/Alphadocte/Game.h" 
    "${SRC_INC_DIR}/Alphadocte/Hint.h"
    "${SRC_INC_DIR}/Alphadocte/IGameRules.h"
    "${SRC_INC_DIR}/Alphadocte/MotusGameRules.h"
    "${SRC_INC_DIR}/Alphadocte/Solver.h"
    "${SRC_INC_DIR}/Alphadocte/TxtDictionary.h"
    "${SRC_INC_DIR}/Alphadocte/WordleGameRules.h"
)

# list source files
set(SRC_FILES
    "${SRC_DIR}/Dictionary.cpp"
    "${SRC_DIR}/EntropyMaximizer.cpp"
    "${SRC_DIR}/Exceptions.cpp"
    "${SRC_DIR}/FixedSizeDictionary.cpp"
    "${SRC_DIR}/Game.cpp"
    "${SRC_DIR}/Hint.cpp"
    "${SRC_DIR}/MotusGameRules.cpp"
    "${SRC_DIR}/Solver.cpp"
    "${SRC_DIR}/TxtDictionary.cpp"
    "${SRC_DIR}/WordleGameRules.cpp"
)

# ship the objects in a common library
add_library(alphadocte "${SRC_FILES}" "${GEN_INC_FILES}" "${SRC_INC_FILES}")
add_library(Alphadocte::Lib ALIAS alphadocte)

# configure library compilation options
target_include_directories(alphadocte PUBLIC ${INC_DIRS})
target_compile_features(alphadocte PUBLIC cxx_std_20)
set_target_properties(alphadocte PROPERTIES CXX_EXTENSIONS OFF)
# Export all symbols of library on Windows to create the associated .lib with the shared .dll
set_target_properties(alphadocte PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Set library version
set_target_properties(alphadocte PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})


# Dependencies
target_link_libraries(alphadocte PUBLIC Boost::boost)

# IDE Support : add include folders
source_group(TREE "${SRC_INC_DIR}" PREFIX "Header Files" FILES ${SRC_INC_FILES})
source_group(TREE "${GEN_INC_DIR}" PREFIX "Header Files" FILES ${GEN_INC_FILES})
source_group(TREE "${SRC_DIR}" PREFIX "Source Files" FILES ${SRC_FILES})
