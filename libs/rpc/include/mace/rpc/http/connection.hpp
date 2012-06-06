#ifndef _MACE_RPC_HTTP_CONNECTION_HPP_
#define _MACE_RPC_HTTP_CONNECTION_HPP_
#include <mace/rpc/http/request.hpp>
#include <mace/rpc/http/reply.hpp>

namespace mace { namespace rpc { namespace http {

  /**
   *  Manages HTTP requests.
   *
   *  Supports keep-alive for sending multiple requests
   *  over the same port.
   */
  class connection {
    public:
      connection( const std::string& host, uint16_t port = 80 );
      ~connection();

      reply send( const request& req ); 

      boost::signal<void()> closed;
    private:
      class connection_private* my;
  };

} } } // mace::rpc::http

#endif // _MACE_RPC_HTTP_CONNECTION_HPP_
