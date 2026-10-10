cmake_minimum_required(VERSION 3.20)
project(deadlocked_native LANGUAGES C CXX)

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
    ../src/game/0045CB88.c
)
target_include_directories(deadlocked_native_core PUBLIC include)
target_compile_features(deadlocked_native_core PUBLIC c_std_11)

add_library(deadlocked_native_scene STATIC scene.cpp)
target_include_directories(deadlocked_native_scene PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_scene PUBLIC cxx_std_17)

if(WIN32)
    add_executable(deadlocked_native_scene_viewer renderer_win.cpp)
    target_link_libraries(deadlocked_native_scene_viewer PRIVATE deadlocked_native_scene
        d3d11 d3dcompiler dxgi windowscodecs ole32 user32)
    target_compile_definitions(deadlocked_native_scene_viewer PRIVATE UNICODE _UNICODE)
endif()

if(BUILD_TESTING)
    add_executable(deadlocked_native_core_tests tests/core_tests.c)
    target_link_libraries(deadlocked_native_core_tests PRIVATE deadlocked_native_core)
    add_test(NAME native_core_abi COMMAND deadlocked_native_core_tests)
    add_executable(deadlocked_native_boot_tests tests/boot_tests.c)
    target_link_libraries(deadlocked_native_boot_tests PRIVATE deadlocked_native_core)
    add_test(NAME native_boot_parameters COMMAND deadlocked_native_boot_tests)
    add_executable(deadlocked_native_scene_tests tests/scene_tests.cpp)
    target_link_libraries(deadlocked_native_scene_tests PRIVATE deadlocked_native_scene)
    add_test(NAME native_scene_validation COMMAND deadlocked_native_scene_tests)
    if(WIN32)
        add_test(NAME native_render_validation COMMAND deadlocked_native_scene_viewer
            --self-test --software --capture ${CMAKE_CURRENT_BINARY_DIR}/test-triangle.png)
        set_tests_properties(native_render_validation PROPERTIES TIMEOUT 30)
    endif()
endif()
