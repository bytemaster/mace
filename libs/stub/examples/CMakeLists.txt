add_executable( stub_rpc_client rpc_client.cpp )
add_executable( stub_rpc_server rpc_server.cpp )

set( libraries pthread 
               ${Boost_ASIO_LIBRARY} 
               ${Boost_SERIALIZATION_LIBRARY}
               ${Boost_SYSTEM_LIBRARY}
               )

target_link_libraries( stub_rpc_client ${libraries} )
target_link_libraries( stub_rpc_server ${libraries} )
