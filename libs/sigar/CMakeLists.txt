include_directories( ../reflect/include )
include_directories( ../rpc/include )
include_directories( ../void/include )
include_directories( ../cmt/include )
include_directories( ../atomic/include )
include_directories( include )
include_directories( ${CMAKE_SOURCE_DIR}/vendor/sigar/include )

SET( libraries mace_sigar sigar mace_rpc ${Boost_FILESYSTEM_LIBRARY} )
IF(APPLE)
	SET( libraries ${libraries} IOKit )
ENDIF(APPLE) 
IF(WIN32 )
	SET( libraries ${libraries} netapi32 version )
ENDIF(WIN32)

SET( sources src/sigar.cpp )

install( DIRECTORY include/mace DESTINATION include/)

SETUP_LIBRARY( mace_sigar SOURCES ${sources}
                        LIBRARIES  ${libraries}
                        LIBRARY_TYPE STATIC )

add_subdirectory(examples)
