include_directories( ../include )
include_directories( ../../atomic/include )
include_directories( ../../cmt/include )
include_directories( ../../move/include )
SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    mace_cmt
    boost_context 
    ${pthread_library}
    ${rt_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

#add_executable( reflect_signal  signal.cpp  )
#target_link_libraries( reflect_signal ${libraries} )
