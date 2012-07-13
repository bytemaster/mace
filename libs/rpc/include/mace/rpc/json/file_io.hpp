#ifndef _MACE_RPC_JSON_FILE_IO_HPP
#define _MACE_RPC_JSON_FILE_IO_HPP
#include <mace/rpc/json/io.hpp>

namespace mace { namespace rpc { namespace json {
  mace::rpc::value from_file( const std::string& local_path );

  template<typename T>
  T from_file( const std::string& local_path ) {
    return mace::rpc::value_cast<T>(mace::rpc::json::from_file(local_path));
  }
} } }

#endif //_MACE_RPC_JSON_FILE_IO_HPP
