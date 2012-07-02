SET( libraries 
    mace_cmt
    boost_context 
    ${pthread_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

add_executable( asio_test asio_test.cpp )
target_link_libraries( asio_test ${libraries} )
add_executable( future_test future_test.cpp )
target_link_libraries( future_test ${libraries} )
