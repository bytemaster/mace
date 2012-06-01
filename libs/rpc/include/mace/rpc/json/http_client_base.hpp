#ifndef _MACE_RPC_JSON_HTTP_CLIENT_BASE_HPP_
#define _MACE_RPC_JSON_HTTP_CLIENT_BASE_HPP_
#include <mace/rpc/json/client_base.hpp>

namespace mace { namespace rpc { namespace json {
  class http_client_base : public client_base {
    public:
      http_client_base( const std::string& url );
      void set_url( const std::string& url );
      void set_http_method( const std::string& method ); // POST / GET 
      void set_path( const std::string& path );
      void set_args( const std::string& args );
      void set_header( const std::string key, const std::string& value );
      boost::optional<std::string> get_header( const std::string& key );
  };
} } }

#endif // _MACE_RPC_JSON_HTTP_CLIENT_BASE_HPP_
