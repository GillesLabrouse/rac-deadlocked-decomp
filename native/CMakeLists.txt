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
    ../src/game/0045C768.c
    ../src/game/0045C8C8.c
    ../src/game/00465C08.c
    ../src/game/004598E0.c
    ../src/game/0047F538.c
    ../src/game/004757A0.c
)
target_compile_definitions(deadlocked_native_core PRIVATE DEADLOCKED_NATIVE=1)
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
    add_executable(deadlocked_native_queue_tests tests/queue_tests.c)
    target_link_libraries(deadlocked_native_queue_tests PRIVATE deadlocked_native_core)
    add_test(NAME native_startup_queue COMMAND deadlocked_native_queue_tests)
    # The handler remains nonmatching; keep its host experiment separate from
    # the accepted matching core and from the scene viewer's product runtime.
    add_executable(deadlocked_native_request_tests tests/request_tests.c
        ../nonmatching/0047F568.c)
    target_compile_definitions(deadlocked_native_request_tests PRIVATE DEADLOCKED_NATIVE=1)
    target_link_libraries(deadlocked_native_request_tests PRIVATE deadlocked_native_core)
    add_test(NAME native_request_draft COMMAND deadlocked_native_request_tests)
    add_executable(deadlocked_native_transition_math_tests tests/transition_math_tests.c)
    target_link_libraries(deadlocked_native_transition_math_tests PRIVATE deadlocked_native_core)
    add_test(NAME native_transition_math COMMAND deadlocked_native_transition_math_tests)
    # Capture unrecovered transition boundaries while exercising the real
    # queue, preflight draft, consumer draft and matching progression helper.
    add_executable(deadlocked_native_consumer_tests tests/consumer_tests.c
        ../nonmatching/0047F568.c ../nonmatching/0047EFA8.c)
    target_compile_definitions(deadlocked_native_consumer_tests PRIVATE DEADLOCKED_NATIVE=1)
    target_link_libraries(deadlocked_native_consumer_tests PRIVATE deadlocked_native_core)
    add_test(NAME native_consumer_draft COMMAND deadlocked_native_consumer_tests)
    add_executable(deadlocked_native_scene_tests tests/scene_tests.cpp)
    target_link_libraries(deadlocked_native_scene_tests PRIVATE deadlocked_native_scene)
    add_test(NAME native_scene_validation COMMAND deadlocked_native_scene_tests)
    if(WIN32)
        add_test(NAME native_render_validation COMMAND deadlocked_native_scene_viewer
            --self-test --software --capture ${CMAKE_CURRENT_BINARY_DIR}/test-triangle.png)
        set_tests_properties(native_render_validation PROPERTIES TIMEOUT 30)
    endif()
endif()
