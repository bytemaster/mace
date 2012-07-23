PROJECT( mace )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8.0 )

SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )

INCLUDE( VersionMacros )

SET( DEFAULT_HEADER_INSTALL_DIR include/\${target} )
SET( DEFAULT_LIBRARY_INSTALL_DIR lib/ )
SET( DEFAULT_EXECUTABLE_INSTALL_DIR bin/ )
SET( CMAKE_DEBUG_POSTFIX _debug )
SET( BUILD_SHARED_LIBS NO )

SET(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(OpenSSL)
FIND_PACKAGE(Boost 1.50 REQUIRED COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework ) 

MESSAGE( STATUS "Boost Include Dir: ${Boost_INCLUDE_DIR}" )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

SET( OPEN_SSL_LIBS ${SSL_EAY_RELEASE} ${LIB_EAY_RELEASE} )

INCLUDE( SetupTargetMacros )

option( ENABLE_EXAMPLES "Build Examples" ON )
option( ENABLE_TESTS "Build Tests" ON )
option( ENABLE_ARM "Build for ARM CPU" OFF )

include_directories(/usr/local/include)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(vendor/libssh2-1.4.2/include)


#ADD_DEFINITIONS( -DBOOST_CONTEXT_UCTX )
#ADD_DEFINITIONS( -D_XOPEN_SOURCE )

IF( WIN32 )
	ADD_DEFINITIONS( -DBOOST_CONTEXT_NO_LIB )
	ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
	ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
	ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
	SET( Rpcrt4 Rpcrt4 )
ELSE(WIN32)
   FIND_PACKAGE(ZLIB)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
#    INCLUDE(CheckCCompilerFlag)
#    CHECK_C_COMPILER_FLAG(-std=c++0x HAVE_CPPOX)
#    IF( HAVE_CPPOX )
#    ELSE( HAVE_CPPOX )
#        MESSAGE( STATUS "DISABLING C++0x Optimizations" )
#        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#    ENDIF( HAVE_CPPOX )
ENDIF(WIN32)

if( UNIX ) 
  SET( TCMALLOC_LIBRARY libtcmalloc.a )
  if( NOT APPLE )  
    set(rt_library rt )
	set(pthread_library pthread) 
  endif() 
endif()


add_subdirectory(libs)
add_subdirectory(vendor)
#add_subdirectory(examples)
