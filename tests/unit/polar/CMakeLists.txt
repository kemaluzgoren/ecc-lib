add_executable(test_polar test_polar.c)
target_link_libraries(test_polar PRIVATE ecc)
target_include_directories(test_polar PRIVATE ${CMAKE_SOURCE_DIR}/src/polar)  # for the internal reliability-table header
add_test(NAME polar_unit_test COMMAND test_polar)
