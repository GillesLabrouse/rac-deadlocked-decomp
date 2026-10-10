cmake_minimum_required(VERSION 3.20)
project(deadlocked_native LANGUAGES C)

include(CTest)

# Explicitly selected portable functions; never link retail machine code.
add_library(deadlocked_native_core STATIC
    ../src/core/00139A58.c
    ../src/core/001585C8.c
    ../src/core/00158630.c
    ../src/core/001586F0.c
    ../src/core/00158780.c
    ../src/core/00158988.c
    ../src/core/001589A8.c
)
target_include_directories(deadlocked_native_core PUBLIC include)
target_compile_features(deadlocked_native_core PUBLIC c_std_11)

if(BUILD_TESTING)
    add_executable(deadlocked_native_core_tests tests/core_tests.c)
    target_link_libraries(deadlocked_native_core_tests PRIVATE deadlocked_native_core)
    add_test(NAME native_core_abi COMMAND deadlocked_native_core_tests)
endif()
