#ifndef _MACE_RPC_HTTP_CONNECTION_HPP_
#define _MACE_RPC_HTTP_CONNECTION_HPP_
#include <mace/rpc/tcp/connection.hpp>
#include <mace/rpc/http/detail/connection.hpp>

namespace mace { namespace rpc { namespace http {
  class request;
  class reply;

  /**
   *  Manages HTTP requests.
   *
   *  Supports keep-alive for sending multiple requests
   *  over the same port.
   */
  template<typename IODelegate=mace::rpc::raw_io>
  class connection : public mace::rpc::tcp::connection<IODelegate> {
    public:
      typedef boost::shared_ptr<connection> ptr;

      template<typename String>
      connection( String&& host, uint16_t port = 80 )
      :rpc::tcp::connection<IODelegate>( 
        new mace::rpc::http::detail::connection(std::forward<String>(host),port) ) {}

      connection( const boost::asio::ip::tcp::endpoint& ep )
      :rpc::tcp::connection<IODelegate>( new mace::rpc::http::detail::connection(ep) ){}

      connection( const mace::cmt::asio::tcp::socket::ptr& sock )
      :rpc::tcp::connection<IODelegate>( new mace::rpc::http::detail::connection(sock) ) {}

      reply send( request&& r ) {
        return static_cast<mace::rpc::http::detail::connection*>(this->my)->send( std::move(r) );
      }

    protected:
      connection( mace::rpc::http::detail::connection* b )
      :rpc::tcp::connection<IODelegate>(b){}
  };

} } } // mace::rpc::http

#endif // _MACE_RPC_HTTP_CONNECTION_HPP_
