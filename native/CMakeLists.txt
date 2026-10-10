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

add_library(deadlocked_native_dreadzone_models STATIC
    ../nonmatching/L01_0058B050.c ../nonmatching/L01_0058B098.c
    ../nonmatching/L01_0058B148.c ../nonmatching/L01_0058B268.c
    ../nonmatching/L01_0058B338.c ../nonmatching/L01_0058B4D0.c)
target_compile_definitions(deadlocked_native_dreadzone_models PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_models PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_models PUBLIC c_std_11)

add_library(deadlocked_native_dreadzone_classes STATIC
    ../nonmatching/L01_005742C8.c ../nonmatching/L01_00576EF8.c
    ../nonmatching/L01_00576BC0.c ../nonmatching/L01_005B5598.c)
target_compile_definitions(deadlocked_native_dreadzone_classes PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_classes PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_classes PUBLIC c_std_11)

add_library(deadlocked_native_dreadzone_geometry STATIC
    ../nonmatching/L01_00576AA8.c ../nonmatching/L01_00576348.c ../nonmatching/L01_00576650.c)
target_compile_definitions(deadlocked_native_dreadzone_geometry PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_geometry PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_geometry PUBLIC c_std_11)
target_link_libraries(deadlocked_native_dreadzone_classes PUBLIC deadlocked_native_dreadzone_geometry)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_memory PRIVATE /W4 /WX)
    target_compile_options(deadlocked_native_dreadzone_payload PRIVATE /W4 /WX)
    target_compile_options(deadlocked_native_dreadzone_models PRIVATE /W4 /WX)
    target_compile_options(deadlocked_native_dreadzone_classes PRIVATE /W4 /WX)
    target_compile_options(deadlocked_native_dreadzone_geometry PRIVATE /W4 /WX)
endif()

# Actual L01 packet decoding and recovered math, kept outside the matching core.
add_library(deadlocked_native_dreadzone_input STATIC
    ../nonmatching/L01_005BDA40.c ../nonmatching/L01_0055D708.c
    ../nonmatching/L01_0055D910.c ../nonmatching/L01_005BD950.c
    ../nonmatching/L01_006B5CB8.c ../nonmatching/L01_006A6598.c
    ../nonmatching/L01_006A93A0.c ../nonmatching/L01_006A9638.c
    ../nonmatching/L01_005BE338.c ../nonmatching/L01_005BE7B0.c
    ../nonmatching/L01_00614EE8.c ../nonmatching/L01_006658C0.c)
target_compile_definitions(deadlocked_native_dreadzone_input PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_input PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_input PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_input PRIVATE /W4 /WX /fp:precise)
endif()

add_library(deadlocked_native_dreadzone_core_startup STATIC
    ../nonmatching/0015A170.c ../nonmatching/0015AA58.c ../nonmatching/0015B498.c)
target_compile_definitions(deadlocked_native_dreadzone_core_startup PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_core_startup PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_core_startup PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_core_startup PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_dreadzone_actor_fields STATIC
    ../nonmatching/L01_0068E3B0.c)
target_compile_definitions(deadlocked_native_dreadzone_actor_fields PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_actor_fields PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_actor_fields PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_actor_fields PRIVATE /W4 /WX /fp:precise)
endif()

add_library(deadlocked_native_dreadzone_actor_snapshot STATIC
    ../nonmatching/L01_0066A2A0.c ../nonmatching/L01_0068DE60.c
    ../nonmatching/L01_006A7D28.c ../nonmatching/L01_00690E68.c)
target_compile_definitions(deadlocked_native_dreadzone_actor_snapshot PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_actor_snapshot PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_actor_snapshot PUBLIC c_std_11)
target_link_libraries(deadlocked_native_dreadzone_actor_snapshot PUBLIC deadlocked_native_dreadzone_input)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_actor_snapshot PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_dreadzone_actor_parameters STATIC
    ../nonmatching/L01_00669A20.c)
target_compile_definitions(deadlocked_native_dreadzone_actor_parameters PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_actor_parameters PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_actor_parameters PUBLIC c_std_11)

# Numeric transfer packets only; execution requires a recovered host graphics path.
add_library(deadlocked_native_texture_transfer_packets STATIC
    ../nonmatching/00138E08.c ../nonmatching/00138FF0.c)
target_compile_definitions(deadlocked_native_texture_transfer_packets PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_texture_transfer_packets PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_texture_transfer_packets PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_actor_parameters PRIVATE /W4 /WX /fp:precise)
    target_compile_options(deadlocked_native_texture_transfer_packets PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_dreadzone_inventory_reset STATIC
    ../nonmatching/L01_006C0D88.c ../nonmatching/L01_0055D2F8.c)
target_compile_definitions(deadlocked_native_dreadzone_inventory_reset PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_inventory_reset PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_inventory_reset PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_inventory_reset PRIVATE /W4 /WX)
endif()

# CPU byte effect of the indexed upload/readback, not GPU submission.
add_library(deadlocked_native_texture_pixels STATIC texture_pixel_permutation.c)
target_include_directories(deadlocked_native_texture_pixels PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_texture_pixels PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_texture_pixels PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_texture_layout STATIC texture_layout_preparation.c)
target_compile_definitions(deadlocked_native_texture_layout PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_texture_layout PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_texture_layout PUBLIC c_std_11)
target_link_libraries(deadlocked_native_texture_layout PUBLIC
    deadlocked_native_texture_pixels deadlocked_native_texture_transfer_packets)
if(MSVC)
    target_compile_options(deadlocked_native_texture_layout PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_dreadzone_actor_animation STATIC
    ../nonmatching/L01_00688FE0.c ../nonmatching/L01_0058A8D8.c ../nonmatching/L01_0058B6B0.c)
target_compile_definitions(deadlocked_native_dreadzone_actor_animation PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_actor_animation PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_actor_animation PUBLIC c_std_11)
target_link_libraries(deadlocked_native_dreadzone_actor_animation PUBLIC deadlocked_native_dreadzone_memory)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_actor_animation PRIVATE /W4 /WX /fp:precise)
endif()

add_library(deadlocked_native_dreadzone_inventory_setup STATIC
    ../nonmatching/L01_005688A0.c ../nonmatching/L01_006BE050.c
    ../nonmatching/L01_006BE198.c ../nonmatching/L01_006BE208.c
    ../nonmatching/L01_006BE240.c ../nonmatching/L01_006BE270.c
    ../nonmatching/L01_006BE480.c ../nonmatching/L01_006BE0D0.c
    ../nonmatching/L01_006BF200.c ../nonmatching/L01_006BF278.c
    ../nonmatching/L01_006BF2D0.c ../nonmatching/L01_006C0090.c
    ../nonmatching/L01_006C01F8.c ../nonmatching/L01_006C04A8.c
    ../nonmatching/L01_006C0740.c ../nonmatching/L01_006C0768.c
    ../nonmatching/L01_006C0C60.c ../nonmatching/L01_006909C8.c)
target_compile_definitions(deadlocked_native_dreadzone_inventory_setup PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_inventory_setup PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_inventory_setup PUBLIC c_std_11)
target_link_libraries(deadlocked_native_dreadzone_inventory_setup PUBLIC deadlocked_native_dreadzone_inventory_reset)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_inventory_setup PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_dreadzone_model_transform STATIC dreadzone_model_transform.c)
target_include_directories(deadlocked_native_dreadzone_model_transform PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_model_transform PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_model_transform PRIVATE /W4 /WX /fp:precise)
endif()

add_library(deadlocked_native_dreadzone_angle_pair STATIC dreadzone_angle_pair.c)
target_include_directories(deadlocked_native_dreadzone_angle_pair PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_angle_pair PUBLIC c_std_11)
add_library(deadlocked_native_dreadzone_camera_init STATIC dreadzone_camera_init.c)
target_include_directories(deadlocked_native_dreadzone_camera_init PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_camera_init PUBLIC c_std_11)
target_link_libraries(deadlocked_native_dreadzone_camera_init PUBLIC deadlocked_native_dreadzone_angle_pair)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_angle_pair PRIVATE /W4 /WX /fp:strict)
    target_compile_options(deadlocked_native_dreadzone_camera_init PRIVATE /W4 /WX /fp:strict)
endif()

add_library(deadlocked_native_dreadzone_vertex_blend STATIC dreadzone_vertex_blend.c)
target_include_directories(deadlocked_native_dreadzone_vertex_blend PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_vertex_blend PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_vertex_blend PRIVATE /W4 /WX /fp:precise)
endif()

add_library(deadlocked_native_dreadzone_spatial_grid STATIC dreadzone_spatial_grid.c)
target_include_directories(deadlocked_native_dreadzone_spatial_grid PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_spatial_grid PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_spatial_grid PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_dreadzone_bind_pose STATIC dreadzone_bind_pose.c)
target_include_directories(deadlocked_native_dreadzone_bind_pose PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_bind_pose PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_bind_pose PRIVATE /W4 /WX /fp:precise)
endif()

add_library(deadlocked_native_dreadzone_equipment STATIC
    ../nonmatching/L01_00690B60.c ../nonmatching/L01_00688578.c
    ../nonmatching/L01_006C0A58.c ../nonmatching/L01_00690C88.c
    ../nonmatching/L01_005E9258.c ../nonmatching/L01_006BDDC0.c
    ../nonmatching/L01_006BE040.c)
target_compile_definitions(deadlocked_native_dreadzone_equipment PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_equipment PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_equipment PUBLIC c_std_11)
target_link_libraries(deadlocked_native_dreadzone_equipment PUBLIC deadlocked_native_dreadzone_inventory_setup)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_equipment PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_dreadzone_equipment_ui STATIC
    dreadzone_equipment_ui.c
    ../nonmatching/L01_005D7558.c ../nonmatching/L01_005E93D8.c
    ../nonmatching/L01_005EA270.c ../nonmatching/L01_006BDDF0.c
    ../nonmatching/L01_005D89E8.c ../nonmatching/L01_005EA230.c
    ../nonmatching/L01_005E97D8.c ../nonmatching/L01_005D8A08.c
    ../nonmatching/L01_005D8A40.c ../nonmatching/L01_005D8AE0.c
    ../nonmatching/L01_005E9350.c ../nonmatching/L01_005EA6A8.c
    ../nonmatching/L01_005F0A40.c ../nonmatching/L01_00652FD0.c)
target_compile_definitions(deadlocked_native_dreadzone_equipment_ui PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_dreadzone_equipment_ui PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_dreadzone_equipment_ui PUBLIC c_std_11)
target_link_libraries(deadlocked_native_dreadzone_equipment_ui PUBLIC
    deadlocked_native_dreadzone_inventory_setup deadlocked_native_dreadzone_input)
if(MSVC)
    target_compile_options(deadlocked_native_dreadzone_equipment_ui PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_game_random STATIC ../nonmatching/00163408.c ../nonmatching/L01_0058FF38.c)
target_compile_definitions(deadlocked_native_game_random PRIVATE DEADLOCKED_NATIVE=1)
target_include_directories(deadlocked_native_game_random PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_game_random PUBLIC c_std_11)
if(MSVC)
    target_compile_options(deadlocked_native_game_random PRIVATE /W4 /WX)
endif()

add_library(deadlocked_native_scene STATIC scene.cpp)
target_include_directories(deadlocked_native_scene PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(deadlocked_native_scene PUBLIC cxx_std_17)

if(WIN32)
    add_executable(deadlocked_native_scene_viewer renderer_win.cpp)
    target_link_libraries(deadlocked_native_scene_viewer PRIVATE deadlocked_native_scene
        d3d11 d3dcompiler dxgi windowscodecs ole32 user32)
    target_compile_definitions(deadlocked_native_scene_viewer PRIVATE UNICODE _UNICODE)
    add_library(deadlocked_native_controller STATIC controller_win.cpp)
    target_compile_features(deadlocked_native_controller PUBLIC cxx_std_17)
    target_include_directories(deadlocked_native_controller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(deadlocked_native_controller PUBLIC deadlocked_native_dreadzone_input xinput)
    add_executable(deadlocked_native_controller_probe controller_probe_win.cpp)
    target_link_libraries(deadlocked_native_controller_probe PRIVATE deadlocked_native_controller)
    if(MSVC)
        target_compile_options(deadlocked_native_controller PRIVATE /W4 /WX)
        target_compile_options(deadlocked_native_controller_probe PRIVATE /W4 /WX)
    endif()
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
    add_executable(deadlocked_native_dreadzone_model_pool_tests tests/dreadzone_model_pool_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_model_pool_tests PRIVATE deadlocked_native_dreadzone_models)
    add_executable(deadlocked_native_dreadzone_class_tables_tests tests/dreadzone_class_tables_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_class_tables_tests PRIVATE deadlocked_native_dreadzone_classes)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_model_pool_tests PRIVATE /W4 /WX)
        target_compile_options(deadlocked_native_dreadzone_class_tables_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_model_pool COMMAND deadlocked_native_dreadzone_model_pool_tests)
    add_test(NAME native_dreadzone_class_tables COMMAND deadlocked_native_dreadzone_class_tables_tests)
    add_executable(deadlocked_native_dreadzone_geometry_tests tests/dreadzone_geometry_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_geometry_tests PRIVATE deadlocked_native_dreadzone_geometry)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_geometry_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_geometry COMMAND deadlocked_native_dreadzone_geometry_tests)
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
    add_executable(deadlocked_native_dreadzone_input_poll_tests tests/dreadzone_input_poll_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_input_poll_tests PRIVATE
        deadlocked_native_dreadzone_input deadlocked_native_dreadzone_memory)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_input_poll_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_input_poll COMMAND deadlocked_native_dreadzone_input_poll_tests)
    add_executable(deadlocked_native_dreadzone_input_repeat_tests tests/dreadzone_input_repeat_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_input_repeat_tests PRIVATE deadlocked_native_dreadzone_input)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_input_repeat_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_input_repeat COMMAND deadlocked_native_dreadzone_input_repeat_tests)
    add_executable(deadlocked_native_dreadzone_core_startup_tests tests/dreadzone_core_startup_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_core_startup_tests PRIVATE deadlocked_native_dreadzone_core_startup)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_core_startup_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_core_startup COMMAND deadlocked_native_dreadzone_core_startup_tests)
    add_executable(deadlocked_native_dreadzone_actor_fields_tests tests/dreadzone_actor_fields_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_actor_fields_tests PRIVATE deadlocked_native_dreadzone_actor_fields)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_actor_fields_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_actor_fields COMMAND deadlocked_native_dreadzone_actor_fields_tests)
    add_executable(deadlocked_native_dreadzone_actor_snapshot_tests tests/dreadzone_actor_snapshot_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_actor_snapshot_tests PRIVATE deadlocked_native_dreadzone_actor_snapshot)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_actor_snapshot_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_actor_snapshot COMMAND deadlocked_native_dreadzone_actor_snapshot_tests)
    add_executable(deadlocked_native_dreadzone_actor_parameters_tests tests/dreadzone_actor_parameters_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_actor_parameters_tests PRIVATE deadlocked_native_dreadzone_actor_parameters)
    add_test(NAME native_dreadzone_actor_parameters COMMAND deadlocked_native_dreadzone_actor_parameters_tests)
    add_executable(deadlocked_native_texture_transfer_packets_tests tests/texture_transfer_packets_tests.c)
    target_link_libraries(deadlocked_native_texture_transfer_packets_tests PRIVATE deadlocked_native_texture_transfer_packets)
    add_test(NAME native_texture_transfer_packets COMMAND deadlocked_native_texture_transfer_packets_tests)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_actor_parameters_tests PRIVATE /W4 /WX /fp:precise)
        target_compile_options(deadlocked_native_texture_transfer_packets_tests PRIVATE /W4 /WX)
    endif()
    add_executable(deadlocked_native_dreadzone_inventory_reset_tests tests/dreadzone_inventory_reset_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_inventory_reset_tests PRIVATE deadlocked_native_dreadzone_inventory_reset)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_inventory_reset_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_inventory_reset COMMAND deadlocked_native_dreadzone_inventory_reset_tests)
    add_executable(deadlocked_native_texture_pixels_tests tests/texture_pixel_permutation_tests.c)
    target_link_libraries(deadlocked_native_texture_pixels_tests PRIVATE deadlocked_native_texture_pixels)
    if(MSVC)
        target_compile_options(deadlocked_native_texture_pixels_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_texture_pixel_permutation COMMAND deadlocked_native_texture_pixels_tests)
    add_executable(deadlocked_native_texture_layout_tests tests/texture_layout_preparation_tests.c)
    target_link_libraries(deadlocked_native_texture_layout_tests PRIVATE deadlocked_native_texture_layout)
    if(MSVC)
        target_compile_options(deadlocked_native_texture_layout_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_texture_layout_preparation COMMAND deadlocked_native_texture_layout_tests)
    add_executable(deadlocked_native_dreadzone_actor_animation_tests tests/dreadzone_actor_animation_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_actor_animation_tests PRIVATE deadlocked_native_dreadzone_actor_animation)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_actor_animation_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_actor_animation COMMAND deadlocked_native_dreadzone_actor_animation_tests)
    add_executable(deadlocked_native_dreadzone_inventory_setup_tests tests/dreadzone_inventory_setup_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_inventory_setup_tests PRIVATE deadlocked_native_dreadzone_inventory_setup)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_inventory_setup_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_inventory_setup COMMAND deadlocked_native_dreadzone_inventory_setup_tests)
    add_executable(deadlocked_native_dreadzone_model_transform_tests tests/dreadzone_model_transform_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_model_transform_tests PRIVATE deadlocked_native_dreadzone_model_transform)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_model_transform_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_model_transform COMMAND deadlocked_native_dreadzone_model_transform_tests)
    add_executable(deadlocked_native_dreadzone_bind_pose_tests tests/dreadzone_bind_pose_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_bind_pose_tests PRIVATE deadlocked_native_dreadzone_bind_pose)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_bind_pose_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_bind_pose COMMAND deadlocked_native_dreadzone_bind_pose_tests)
    add_executable(deadlocked_native_dreadzone_camera_init_tests tests/dreadzone_camera_init_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_camera_init_tests PRIVATE deadlocked_native_dreadzone_camera_init)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_camera_init_tests PRIVATE /W4 /WX /fp:strict)
    endif()
    add_test(NAME native_dreadzone_camera_init COMMAND deadlocked_native_dreadzone_camera_init_tests)
    add_executable(deadlocked_native_dreadzone_vertex_blend_tests tests/dreadzone_vertex_blend_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_vertex_blend_tests PRIVATE deadlocked_native_dreadzone_vertex_blend)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_vertex_blend_tests PRIVATE /W4 /WX /fp:precise)
    endif()
    add_test(NAME native_dreadzone_vertex_blend COMMAND deadlocked_native_dreadzone_vertex_blend_tests)
    add_executable(deadlocked_native_dreadzone_spatial_grid_tests tests/dreadzone_spatial_grid_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_spatial_grid_tests PRIVATE deadlocked_native_dreadzone_spatial_grid)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_spatial_grid_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_spatial_grid COMMAND deadlocked_native_dreadzone_spatial_grid_tests)
    add_executable(deadlocked_native_dreadzone_equipment_tests tests/dreadzone_equipment_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_equipment_tests PRIVATE deadlocked_native_dreadzone_equipment)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_equipment_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_equipment COMMAND deadlocked_native_dreadzone_equipment_tests)
    add_executable(deadlocked_native_dreadzone_equipment_ui_tests tests/dreadzone_equipment_ui_tests.c)
    target_link_libraries(deadlocked_native_dreadzone_equipment_ui_tests PRIVATE deadlocked_native_dreadzone_equipment_ui)
    if(MSVC)
        target_compile_options(deadlocked_native_dreadzone_equipment_ui_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_dreadzone_equipment_ui COMMAND deadlocked_native_dreadzone_equipment_ui_tests)
    add_executable(deadlocked_native_game_random_tests tests/game_random_tests.c)
    target_link_libraries(deadlocked_native_game_random_tests PRIVATE deadlocked_native_game_random)
    if(MSVC)
        target_compile_options(deadlocked_native_game_random_tests PRIVATE /W4 /WX)
    endif()
    add_test(NAME native_game_random COMMAND deadlocked_native_game_random_tests)
    add_executable(deadlocked_native_scene_tests tests/scene_tests.cpp)
    target_link_libraries(deadlocked_native_scene_tests PRIVATE deadlocked_native_scene)
    add_test(NAME native_scene_validation COMMAND deadlocked_native_scene_tests)
    if(WIN32)
        add_executable(deadlocked_native_controller_tests tests/controller_win_tests.cpp)
        target_link_libraries(deadlocked_native_controller_tests PRIVATE deadlocked_native_controller)
        if(MSVC)
            target_compile_options(deadlocked_native_controller_tests PRIVATE /W4 /WX)
        endif()
        add_test(NAME native_controller_adapter COMMAND deadlocked_native_controller_tests)
        add_test(NAME native_render_validation COMMAND deadlocked_native_scene_viewer
            --self-test --software --capture ${CMAKE_CURRENT_BINARY_DIR}/test-triangle.png)
        set_tests_properties(native_render_validation PROPERTIES TIMEOUT 30)
    endif()
endif()
