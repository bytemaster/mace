include_directories( include )
include_directories( ../json/include )
include_directories( ../cmt/include )
include_directories( ../stub/include )
include_directories( ../reflect/include )
include_directories( ../context/include )
include_directories( ../atomic/include )
include_directories( ../void/include )
include_directories( ../process/include )
include_directories( ${Boost_INCLUDE_DIR} )

SET( libraries  mace_cmt ssh2)
SET( sources src/client.cpp src/process.cpp )

install( DIRECTORY include/mace/ssh DESTINATION include/mace )

SETUP_LIBRARY( mace_ssh SOURCES ${sources}
                        LIBRARIES  ${libraries}
                        LIBRARY_TYPE STATIC )
 
add_subdirectory(examples)
