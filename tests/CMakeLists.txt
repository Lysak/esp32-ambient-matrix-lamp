add_executable(test_xy_mapping test_xy_mapping.cpp)
target_link_libraries(test_xy_mapping PRIVATE effects-core)

enable_testing()
add_test(NAME xy_mapping COMMAND test_xy_mapping)
