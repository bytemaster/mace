include_directories( ../reflect/include )
include_directories( ../rpc/include )
include_directories( ../void/include )
include_directories( ../cmt/include )
include_directories( include )

SET( libraries mace_sigar -lsigar mace_rpc ${Boost_FILESYSTEM_LIBRARY} IOKit )
SET( sources src/sigar.cpp )

install( DIRECTORY include/mace DESTINATION include/mace )

SETUP_LIBRARY( mace_sigar SOURCES ${sources}
                        LIBRARIES  ${libraries}
                        LIBRARY_TYPE STATIC )

add_subdirectory(examples)
