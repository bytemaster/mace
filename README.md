# Massively Asynchronous Coding Environment 
-----------------------------------------

This project is a collection of tools to enable the development of asynchronous
applications.  This includes utilities for:
  
  1. Multi-tasking and Multi-threading via Boost.CMT
  2. Remote Procedure Calls via Boost.RPC
  3. Actor Patterns made possible by Boost.Reflect 
  4. Console tools that provide better debugging 
      * color coded console, with automatic thread/file/line/function tags.
  5. 3rd party sandboxed boost libraries like Boost.Context

### Installation 

    git clone https://github.com/bytemaster/mace
    cd mace
    git submodule init
    git submodule update
    cmake .
    make
    make install


### Notice 

    These libraries are not part of the official Boost C++ library, but
    is written, to the best of my ability, to follow the best practices
    established by the Boost community and with the hope of being 
    considered for inclusion with a future Boost release.
