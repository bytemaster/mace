SET( libraries 
    mace_sigar
    ${pthread_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

add_executable( stat  stat.cpp  )
target_link_libraries( stat ${libraries} )
