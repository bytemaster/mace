SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    mace_cmt
    mace_rpc
    boost_context 
    ${pthread_library}
    ${rt_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

IF(UNIX) 
    set( libraries ${libraries} pthread )
ENDIF(UNIX)
#add_executable( httpc  httpc.cpp  )
#target_link_libraries( httpc ${libraries} )
#add_executable( https  https.cpp  )
#target_link_libraries( https ${libraries} )

#add_executable( jsonv  jsonv.cpp  )
#target_link_libraries( jsonv ${libraries} )

add_executable( tcpc  tcpc.cpp  )
target_link_libraries( tcpc ${libraries} )
add_executable( tcps  tcps.cpp  )
target_link_libraries( tcps ${libraries} )

add_executable( udpc  udpc.cpp  )
target_link_libraries( udpc ${libraries} )
add_executable( udps  udps.cpp  )
target_link_libraries( udps ${libraries} )
#
#add_executable( rpcs  rpc_server.cpp  )
#target_link_libraries( rpcs ${libraries} )
#
#add_executable( rpcc  rpc_client.cpp  )
#target_link_libraries( rpcc ${libraries} )
#
#add_executable( benchc  benchc.cpp  )
#target_link_libraries( benchc ${libraries} )
#
#add_executable( jsonhttpd  jsonhttpd.cpp  )
#target_link_libraries( jsonhttpd ${libraries} cppnetlib-uri cppnetlib-server-parsers )
#add_executable( jsonhttpc  jsonhttpc.cpp  )
#target_link_libraries( jsonhttpc ${libraries} cppnetlib-uri cppnetlib-server-parsers cppnetlib-client-connections)

#add_executable( jsonc  json_client.cpp  )
#target_link_libraries( jsonc ${libraries} )
#add_executable( jsons  json_echo_server.cpp  )
#target_link_libraries( jsons ${libraries} )
