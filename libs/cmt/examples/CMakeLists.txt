

SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    mace_cmt
    boost_context 
    ${Rpcrt4}
    ${pthread_library}
    ${rt_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_FILESYSTEM_LIBRARY} 
    ${Boost_CHRONO_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})
    
#add_executable( rvwrap  rvwrap.cpp o.cpp  )
#target_link_libraries( rvwrap ${libraries} )

add_executable( actor  actor.cpp  )
add_executable( cmt_test test.cpp  )
add_executable( mutex mutex.cpp  )
add_executable( run run.cpp  )
#add_executable( thread_test thread_test.cpp  )



target_link_libraries( run ${libraries} )
target_link_libraries( actor ${libraries} )
target_link_libraries( cmt_test ${libraries} )
target_link_libraries( mutex ${libraries} )
#target_link_libraries( thread_test ${libraries} )

