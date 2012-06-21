include_directories( include )
include_directories( ../void/include )
install( DIRECTORY include/mace/stub DESTINATION include/mace )
add_subdirectory(examples)
