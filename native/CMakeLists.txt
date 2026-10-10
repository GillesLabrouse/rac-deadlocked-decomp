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
    ../src/game/00510EB0.c
)
target_compile_definitions(deadlocked_native_core PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_core PUBLIC include)
target_compile_features(deadlocked_native_core PUBLIC c_std_11)

add_library(deadlocked_native_dreadzone_memory STATIC
    ../nonmatching/L01_00628020.c ../nonmatching/L01_006A9640.c
    ../nonmatching/L01_006AF588.c ../nonmatching/L01_00529280.c
    ../nonmatching/L01_0055DFE8.c ../nonmatching/L01_0055DCF8.c
    ../nonmatching/0011A518.c ../nonmatching/L01_005D6CB0.c
    ../nonmatching/L01_00666600.c)
target_compile_definitions(deadlocked_native_dreadzone_memory PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_memory PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_memory PUBLIC c_std_11)

# Payload descriptors preserve guest32 links; the actor constructor is separate.
add_library(deadlocked_native_dreadzone_payload STATIC
    ../nonmatching/L01_0058D538.c ../nonmatching/L01_0058D6A8.c
    ../nonmatching/L01_00586B10.c ../nonmatching/L01_00589210.c
    ../nonmatching/L01_0058D468.c)
target_compile_definitions(deadlocked_native_dreadzone_payload PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_payload PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_payload PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_memory PRIVATE /W4 /WX)
    target_compile_options(deadlocked_native_dreadzone_payload PRIVATE /W4 /WX)
endif()

# Actual L01 packet decoding and recovered math, kept outside the matching core.
add_library(deadlocked_native_dreadzone_input STATIC
    ../nonmatching/L01_005BDA40.c ../nonmatching/L01_0055D708.c
    ../nonmatching/L01_0055D910.c ../nonmatching/L01_005BD950.c
    ../nonmatching/L01_006B5CB8.c ../nonmatching/L01_006A6598.c
    ../nonmatching/L01_006A93A0.c ../nonmatching/L01_006A9638.c)
target_compile_definitions(deadlocked_native_dreadzone_input PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_input PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_input PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_input PRIVATE /W4 /WX /fp:precise)
endif()

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
    # Execute actual L01 memory algorithms in isolated guest-layout storage.
    add_executable(deadlocked_native_dreadzone_memory_tests
        tests/dreadzone_memory_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_memory_tests PRIVATE deadlocked_native_dreadzone_memory)
    add_test(NAME native_dreadzone_memory COMMAND deadlocked_native_dreadzone_memory_tests)
    add_executable(deadlocked_native_dreadzone_record_reset_tests
        tests/dreadzone_record_reset_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_record_reset_tests PRIVATE deadlocked_native_dreadzone_memory)
    add_test(NAME native_dreadzone_record_reset COMMAND deadlocked_native_dreadzone_record_reset_tests)
    add_executable(deadlocked_native_dreadzone_bulk_clear_tests tests/dreadzone_bulk_clear_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_bulk_clear_tests PRIVATE deadlocked_native_dreadzone_memory)
    add_test(NAME native_dreadzone_bulk_clear COMMAND deadlocked_native_dreadzone_bulk_clear_tests)
    add_executable(deadlocked_native_dreadzone_state_init_tests tests/dreadzone_state_init_tests.c)
    target_compile_definitions(deadlocked_native_dreadzone_state_init_tests PRIVATE DEADLOCKED_NATIVE=1)
    target_link_libraries(deadlocked_native_dreadzone_state_init_tests PRIVATE deadlocked_native_dreadzone_memory)
    add_test(NAME native_dreadzone_state_init COMMAND deadlocked_native_dreadzone_state_init_tests)
    add_executable(deadlocked_native_dreadzone_byte_fill_tests tests/dreadzone_byte_fill_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_byte_fill_tests PRIVATE deadlocked_native_dreadzone_memory deadlocked_native_core)
    add_test(NAME native_dreadzone_byte_fill COMMAND deadlocked_native_dreadzone_byte_fill_tests)
    add_executable(deadlocked_native_dreadzone_actor_pool_tests tests/dreadzone_actor_pool_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_actor_pool_tests PRIVATE deadlocked_native_dreadzone_payload)
    add_test(NAME native_dreadzone_actor_pool COMMAND deadlocked_native_dreadzone_actor_pool_tests)
    add_executable(deadlocked_native_dreadzone_actor_class_tests tests/dreadzone_actor_class_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_actor_class_tests PRIVATE deadlocked_native_dreadzone_payload)
    add_test(NAME native_dreadzone_actor_class COMMAND deadlocked_native_dreadzone_actor_class_tests)
    add_executable(deadlocked_native_dreadzone_input_tests tests/dreadzone_input_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_input_tests PRIVATE deadlocked_native_dreadzone_input)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_input_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_input COMMAND deadlocked_native_dreadzone_input_tests)
    add_executable(deadlocked_native_dreadzone_input_reset_tests tests/dreadzone_input_reset_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_input_reset_tests PRIVATE deadlocked_native_dreadzone_input)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_input_reset_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_input_reset COMMAND deadlocked_native_dreadzone_input_reset_tests)
    add_executable(deadlocked_native_dreadzone_input_callback_tests tests/dreadzone_input_callback_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_input_callback_tests PRIVATE deadlocked_native_dreadzone_input)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_input_callback_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_input_callback COMMAND deadlocked_native_dreadzone_input_callback_tests)
    add_executable(deadlocked_native_dreadzone_input_context_init_tests tests/dreadzone_input_context_init_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_input_context_init_tests PRIVATE
        deadlocked_native_dreadzone_input deadlocked_native_dreadzone_memory)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_input_context_init_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_input_context_init COMMAND deadlocked_native_dreadzone_input_context_init_tests)
    add_executable(deadlocked_native_scene_tests tests/scene_tests.cpp)
    target_link_libraries(deadlocked_native_scene_tests PRIVATE deadlocked_native_scene)
    add_test(NAME native_scene_validation COMMAND deadlocked_native_scene_tests)
    if(WIN32)
        add_test(NAME native_render_validation COMMAND deadlocked_native_scene_viewer
            --self-test --software --capture ${CMAKE_CURRENT_BINARY_DIR}/test-triangle.png)
        set_tests_properties(native_render_validation PROPERTIES TIMEOUT 30)
    endif()
endif()
