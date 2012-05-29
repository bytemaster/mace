

SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    mace_cmt
    boost_context 
    pthread
    ${rt_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_CHRONO_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})
    

add_executable( actor  actor.cpp  )
add_executable( cmt_test test.cpp  )
#add_executable( thread_test thread_test.cpp  )


target_link_libraries( actor ${libraries} )
target_link_libraries( cmt_test ${libraries} )
#target_link_libraries( thread_test ${libraries} )

