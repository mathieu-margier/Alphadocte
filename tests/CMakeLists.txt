set(APP_SRC_FOLDER "${PROJECT_SOURCE_DIR}/apps")

set(SRC_FILES
    MainTests.cpp
    DictionaryTests.cpp
    EntropySolverTests.cpp
    GameRulesTests.cpp
    GameTests.cpp
    HintTests.cpp
    SolverTests.cpp
    cli/CacheConfigTests.cpp
    cli/CommonTests.cpp
    cli/ConfigTests.cpp
    stubs/DictionaryStub.cpp
    stubs/DictionaryStub.h
    stubs/SolverStub.cpp
    stubs/SolverStub.h
    # CLI files
    "${APP_SRC_FOLDER}/CacheConfig.cpp"
    "${APP_SRC_FOLDER}/CacheConfig.h"
    "${APP_SRC_FOLDER}/Common.cpp"
    "${APP_SRC_FOLDER}/Common.h"
    "${APP_SRC_FOLDER}/Config.cpp"
    "${APP_SRC_FOLDER}/Config.h"
)

add_executable(alphadocte-tests "${SRC_FILES}")
add_executable(Alphadocte::Tests ALIAS alphadocte-tests)

target_link_libraries(alphadocte-tests PRIVATE Catch2::Catch2 termcolor::termcolor Alphadocte::Lib)
set_target_properties(alphadocte-tests PROPERTIES CXX_EXTENSIONS OFF)

# copy tests data files
add_custom_command(TARGET alphadocte-tests 
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:alphadocte-tests>/data")

catch_discover_tests(alphadocte-tests WORKING_DIRECTORY "$<TARGET_FILE_DIR:alphadocte-tests>")
