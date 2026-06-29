add_executable(test_xy_mapping test_xy_mapping.cpp)
target_link_libraries(test_xy_mapping PRIVATE effects-core)

add_executable(test_effect_color_source test_effect_color_source.cpp)
target_link_libraries(test_effect_color_source PRIVATE effects-core)

enable_testing()
add_test(NAME xy_mapping COMMAND test_xy_mapping)
add_test(NAME effect_color_source COMMAND test_effect_color_source)
