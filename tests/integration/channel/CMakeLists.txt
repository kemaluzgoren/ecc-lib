add_executable(test_channel_bsc_sim test_channel_bsc_sim.c)
target_link_libraries(test_channel_bsc_sim PRIVATE ecc)
if(UNIX)
    target_link_libraries(test_channel_bsc_sim PRIVATE m)  # fabs() needs libm on Linux/gcc
endif()
add_test(NAME channel_bsc_integration_test COMMAND test_channel_bsc_sim)