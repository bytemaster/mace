include_directories( include )
include_directories( ../json/include )
include_directories( ../cmt/include )
include_directories( ../stub/include )
include_directories( ../reflect/include )
include_directories( ../context/include )
include_directories( ../atomic/include )
include_directories( ../void/include )
include_directories( ../process/include )
include_directories( ../ssh/include )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
#include_directories( ../network/include )

IF( ENABLE_EXAMPLES MATCHES ON )
add_subdirectory( examples )
ENDIF()
IF( ENABLE_TESTS MATCHES ON )
ENDIF()
add_subdirectory( tests )

SET( libraries  mace_cmt ${Boost_FILESYSTEM_LIBRARY})
SET( sources 
#        src/connection_base.cpp
#        src/pipe/connection.cpp
        src/json/io.cpp
        src/value.cpp
#        src/json/message.cpp
#        src/json/tcp/connection.cpp
#        src/json/pipe/connection.cpp
#        src/json/message.cpp
#        src/json/tcp/connection.cpp

#        src/json/connection.cpp
#        src/json/tcp_connection.cpp
#        src/json/client_base.cpp
#        src/json/http_client_base.cpp
#        src/json/tcp_server.cpp
#        src/json/http_server.cpp
#        src/json/http_connection.cpp

#        src/http/file_handler.cpp
#        src/http/path_handler.cpp
#        src/http/mime_types.cpp
#        src/http/reply.cpp
#        src/http/request_parser.cpp
#        src/http/server.cpp
             )

install( DIRECTORY include/mace/rpc DESTINATION include/mace )

SETUP_LIBRARY( mace_rpc SOURCES ${sources}
                         LIBRARIES  ${libraries}
                         LIBRARY_TYPE STATIC )
 

