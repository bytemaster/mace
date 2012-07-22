SET( libraries 
    mace_sigar
    mace_cmt
    ${pthread_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

IF(APPLE)
  FIND_LIBRARY( CORE_FOUNDATION CoreFoundation )
  FIND_LIBRARY( IO_KIT IOKit )
  FIND_LIBRARY( CORE_SERVICES CoreServices )
	SET( libraries ${libraries} ${IO_KIT} ${CORE_FOUNDATION} ${CORE_SERVICES} )
ENDIF(APPLE) 

add_executable( stat  stat.cpp  )
target_link_libraries( stat ${libraries} )
