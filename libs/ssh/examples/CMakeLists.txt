SET( libraries 
    ${LIB_EAY_RELEASE} 
    ${SSL_EAY_RELEASE}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${ZLIB_LIBRARY}
    mace_cmt
    mace_ssh
    boost_context 
    ${pthread_library}
    ${rt_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_CHRONO_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_FILESYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY}
    ${LIB_EAY_DEBUG}
    ${LIB_EAY_RELEASE}
    ${SSL_EAY_DEBUG}
    ${SSL_EAY_RELEASE}
    )
MESSAGE( STATUS "Filesystem: ${Boost_FILESYSTEM_LIBRARY}" )

#add_executable( interact  interact.cpp  )
#target_link_libraries( interact ${libraries} )
add_executable( ssh_ls  ssh_ls.cpp  )
target_link_libraries( ssh_ls ${libraries} )

