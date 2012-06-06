include_directories( ../libs/cmt/include )
include_directories( ../libs/reflect/include )
include_directories( ../libs/atomic/include )

SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    mace_cmt
    boost_context 
    tcmalloc
    ${pthread_library}
    ${rt_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_CHRONO_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})


add_executable( cmt_mt  cmt_mt.cpp  )
target_link_libraries( cmt_mt ${libraries} )

add_executable( cmt_st  cmt_st.cpp  )
target_link_libraries( cmt_st ${libraries} )

add_executable( cmt_it  cmt_it.cpp  )
target_link_libraries( cmt_it ${libraries} )

add_executable( cmt_signal  cmt_signal.cpp  )
target_link_libraries( cmt_signal ${libraries} )

add_executable( reflect_to_json  reflect_to_json.cpp  )
target_link_libraries( reflect_to_json ${libraries} )
