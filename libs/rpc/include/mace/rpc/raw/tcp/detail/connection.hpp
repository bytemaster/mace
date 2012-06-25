#ifndef _MACE_RPC_RAW_TCP_DETAIL_CONNECTION_HPP
#define _MACE_RPC_RAW_TCP_DETAIL_CONNECTION_HPP
#include <mace/rpc/tcp/detail/connection.hpp>
#include <mace/rpc/raw/message.hpp>
#include <mace/rpc/raw/raw_io.hpp>

namespace mace { namespace rpc { namespace raw { namespace tcp {  namespace detail {

  class connection : public mace::rpc::tcp::detail::connection  {
    public:
      connection(){}

      connection( const boost::asio::ip::tcp::endpoint& ep )
      :mace::rpc::tcp::detail::connection(ep) {}

      connection( const mace::cmt::asio::tcp::socket::ptr& sock )
      :mace::rpc::tcp::detail::connection(sock) {}

      virtual void  send_message( rpc::message&& m ) {
        raw::message rm( std::move(m) );
        raw::pack( *m_sock, rm );
      }
      virtual rpc::message read_message() {
        raw::message rm;
        raw::unpack( *m_sock, rm );
        return static_cast<rpc::message>(rm);
      }
  };

} } } } } // mace::rpc::raw::tcp::deatil

#endif // _MACE_RPC_RAW_TCP_DETAIL_CONNECTION_HPP
