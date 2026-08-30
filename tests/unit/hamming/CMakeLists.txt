add_executable(test_hamming test_hamming.c)
target_link_libraries(test_hamming PRIVATE ecc)
add_test(NAME hamming_unit_test COMMAND test_hamming)