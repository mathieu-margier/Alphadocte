# specify src and include folders
set(SRC_DIR "${PROJECT_SOURCE_DIR}/apps")
set(INC_DIR "${PROJECT_SOURCE_DIR}/apps")

# list source and header files

# solver files
set(SOLVER_INC_FILES
    "${INC_DIR}/CacheConfig.h"
    "${INC_DIR}/Common.h"
    "${INC_DIR}/Config.h"
)

set(SOLVER_SRC_FILES
    "${SRC_DIR}/CacheConfig.cpp"
    "${SRC_DIR}/Common.cpp"
    "${SRC_DIR}/Config.cpp"
    "${SRC_DIR}/SolverCLI.cpp"
)

# players files
set(PLAYER_INC_FILES
    "${INC_DIR}/Common.h"
)

set(PLAYER_SRC_FILES
    "${SRC_DIR}/Common.cpp"
    "${SRC_DIR}/PlayerCLI.cpp"
)

# build the player and solver executables
add_executable(alphadocte-solver "${SOLVER_SRC_FILES}" "${SOLVER_INC_FILES}")
add_executable(alphadocte-player "${PLAYER_SRC_FILES}" "${PLAYER_INC_FILES}")
add_executable(Alphadocte::Solver ALIAS alphadocte-solver)
add_executable(Alphadocte::Player ALIAS alphadocte-player)

# configure executable compilation options
target_link_libraries(alphadocte-player PRIVATE Alphadocte::Lib $<BUILD_INTERFACE:termcolor::termcolor>)
target_compile_features(alphadocte-player PRIVATE cxx_std_20)
set_target_properties(alphadocte-player PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(alphadocte-solver PRIVATE Alphadocte::Lib $<BUILD_INTERFACE:termcolor::termcolor>)
target_compile_features(alphadocte-solver PRIVATE cxx_std_20)
set_target_properties(alphadocte-solver PROPERTIES CXX_EXTENSIONS OFF)

# IDE Support : add include folders
source_group(TREE "${INC_DIR}" PREFIX "Solver/Header Files" FILES ${SOLVER_INC_FILES})
source_group(TREE "${SRC_DIR}" PREFIX "Solver/Source Files" FILES ${SOLVER_SRC_FILES})
source_group(TREE "${INC_DIR}" PREFIX "Player/Header Files" FILES ${PLAYER_INC_FILES})
source_group(TREE "${SRC_DIR}" PREFIX "Player/Source Files" FILES ${PLAYER_SRC_FILES})
