#ifndef _MACE_RPC_RAW_TCP_DETAIL_CONNECTION_HPP
#define _MACE_RPC_RAW_TCP_DETAIL_CONNECTION_HPP
#include <mace/rpc/tcp/detail/connection.hpp>
#include <mace/rpc/raw/message.hpp>
#include <mace/rpc/raw/raw_io.hpp>

namespace mace { namespace rpc { namespace json { namespace tcp {  namespace detail {

  class connection : public mace::rpc::tcp::detail::connection  {
    public:
      connection();
      connection( const boost::asio::ip::tcp::endpoint& ep );
      connection( const mace::cmt::asio::tcp::socket::ptr& sock );

      virtual void         send_message( rpc::message&& m );
      virtual rpc::message read_message();
  };

} } } } } // mace::rpc::raw::tcp::deatil

#endif // _MACE_RPC_RAW_TCP_DETAIL_CONNECTION_HPP
