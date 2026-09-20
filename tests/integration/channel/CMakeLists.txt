add_executable(test_channel_bsc_sim test_channel_bsc_sim.c)
target_link_libraries(test_channel_bsc_sim PRIVATE ecc)
if(UNIX)
    target_link_libraries(test_channel_bsc_sim PRIVATE m)  # fabs() needs libm on Linux/gcc
endif()
add_test(NAME channel_bsc_integration_test COMMAND test_channel_bsc_sim)

add_executable(test_channel_awgn_sim test_channel_awgn_sim.c)
target_link_libraries(test_channel_awgn_sim PRIVATE ecc)
if(UNIX)
    target_link_libraries(test_channel_awgn_sim PRIVATE m)  # erfc/fabs/sqrt need libm on Linux/gcc
endif()
add_test(NAME channel_awgn_integration_test COMMAND test_channel_awgn_sim)