SET( libraries 
    mace_rpc
    boost_context 
    mace_ssh
    ${pthread_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

add_executable( value_test value_test.cpp )
target_link_libraries( value_test ${libraries} )
add_executable( raw_test raw_test.cpp )
target_link_libraries( raw_test ${libraries} )
add_executable( runner runner.cpp )
target_link_libraries( runner ${libraries} )
add_executable( pipe_test pipe_test.cpp )
target_link_libraries( pipe_test ${libraries} )
add_executable( ssh_test ssh_test.cpp )
target_link_libraries( ssh_test ${libraries} )
