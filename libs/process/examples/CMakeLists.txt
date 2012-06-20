include_directories( ../include )

SET( libraries ${pthread_library} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} )

add_executable( async_io async_io.cpp )
target_link_libraries( async_io ${libraries} )

add_executable( async_wait async_wait.cpp )
target_link_libraries( async_wait ${libraries} )

add_executable( chroot_setup chroot_setup.cpp )
target_link_libraries( chroot_setup ${libraries} )

add_executable( create_child create_child.cpp )
target_link_libraries( create_child ${libraries} )
