SET( libraries 
    mace_cmt
    mace_rpc
    boost_context 
    ${Rpcrt4}
    ${pthread_library}
    ${rt_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_FILESYSTEM_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

#add_executable( httpc  httpc.cpp  )
#target_link_libraries( httpc ${libraries} )
#add_executable( https  https.cpp  )
#target_link_libraries( https ${libraries} )

#add_executable( jsonv  jsonv.cpp  )
#target_link_libraries( jsonv ${libraries} )

#add_executable( json_io json_io.cpp )
#target_link_libraries(json_io ${libraries} )

add_executable( parent  parent.cpp  )
add_executable( child  child.cpp  )
target_link_libraries( parent ${libraries} )
target_link_libraries( child ${libraries} )

#add_executable( tcpc  tcpc.cpp  )
#target_link_libraries( tcpc ${libraries} )
#add_executable( tcps  tcps.cpp  )
#target_link_libraries( tcps ${libraries} )

#add_executable( json_tcpc  json_tcpc.cpp  )
#target_link_libraries( json_tcpc ${libraries} )
#add_executable( json_tcps  json_tcps.cpp  )
#target_link_libraries( json_tcps ${libraries} )
#add_executable( udpc  udpc.cpp  )
#target_link_libraries( udpc ${libraries} )
#add_executable( udps  udps.cpp  )
#target_link_libraries( udps ${libraries} )
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
