#ifndef _MACE_RPC_JSON_TCP_DETAIL_CONNECTION_HPP
#define _MACE_RPC_JSON_TCP_DETAIL_CONNECTION_HPP
#include <mace/rpc/tcp/detail/connection.hpp>
#include <mace/rpc/raw/message.hpp>
#include <mace/rpc/raw/raw_io.hpp>

namespace mace { namespace rpc { namespace json { namespace tcp {  namespace detail {

  class connection : public mace::rpc::tcp::detail::connection  {
    public:
      connection( mace::rpc::connection_base& s );
      connection( mace::rpc::connection_base& s, const boost::asio::ip::tcp::endpoint& ep );
      connection( mace::rpc::connection_base& s, const mace::cmt::asio::tcp::socket::ptr& sock );

      virtual void         send_message( rpc::message&& m );
      virtual rpc::message read_message();
  };

} } } } } // mace::rpc::raw::tcp::deatil

#endif // _MACE_RPC_RAW_TCP_DETAIL_CONNECTION_HPP
