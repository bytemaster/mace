#ifndef _MACE_RPC_JSON_FILE_IO_HPP
#define _MACE_RPC_JSON_FILE_IO_HPP
#include <mace/rpc/json/io.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/filesystem.hpp>
#include <mace/rpc/error.hpp>

namespace mace { namespace rpc { namespace json {
  template<typename T>
  T from_file( const std::string& local_path ) {
    using namespace boost::filesystem;
    if( !exists(local_path) ) {
      MACE_RPC_THROW( "Source file '%1%' does not exist", %local_path );
    }
    if( is_directory( local_path ) ) {
      MACE_RPC_THROW( "Source path '%1%' is a directory, expected a file.", %local_path );
    }

    using namespace boost::interprocess;
    // memory map the file
    file_mapping fmap( local_path.c_str(), read_only );
    size_t       fsize = file_size(local_path);


    mapped_region mr( fmap, boost::interprocess::read_only, 0, fsize );

    const char* pos = reinterpret_cast<const char*>(mr.get_address());
    const char* end = pos + fsize;

    // TODO: implement a const version of to_value 
    std::vector<char> tmp(pos,end);

    json::error_collector ec;
    return mace::rpc::value_cast<T>(mace::rpc::json::to_value(&tmp.front(),&tmp.front()+fsize,ec));
  }
} } }

#endif //_MACE_RPC_JSON_FILE_IO_HPP
