
include_directories( include )
include_directories( ../context/include )
include_directories( ../atomic/include )
include_directories( ../reflect/include )
include_directories( ../stub/include )
include_directories( ../void/include )

SET( sources 
            src/future.cpp
            src/thread.cpp
            src/mutex.cpp
            src/asio.cpp
            src/stdin.cpp
            src/asio/acceptor.cpp
            src/asio/udp/socket.cpp
            src/asio/tcp/socket.cpp
             )


SET( libraries 
    boost_context 
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_CHRONO_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})
SETUP_LIBRARY( mace_cmt SOURCES ${sources}
                             LIBRARIES  ${libraries}
                             LIBRARY_TYPE STATIC )

install( DIRECTORY include/mace/cmt DESTINATION include/mace )
add_subdirectory( examples )
add_subdirectory( tests )
