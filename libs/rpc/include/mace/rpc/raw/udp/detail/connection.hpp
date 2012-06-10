#ifndef _MACE_RPC_RAW_UDP_DETAIL_CONNECTION_HPP
#define _MACE_RPC_RAW_UDP_DETAIL_CONNECTION_HPP
#include <mace/rpc/udp/detail/connection.hpp>
#include <mace/rpc/raw/message.hpp>
#include <mace/rpc/raw/raw_io.hpp>

namespace mace { namespace rpc { namespace raw { namespace udp {  namespace detail {

  class connection : public mace::rpc::udp::detail::connection  {
    public:
      connection(){}

      connection( const boost::asio::ip::udp::endpoint& ep )
      :mace::rpc::udp::detail::connection(ep) {}

      connection( const mace::rpc::udp::socket_ptr& sock )
      :mace::rpc::udp::detail::connection(sock) {}

      virtual void  send_message( rpc::message&& m ) {
        raw::message rm( std::move(m) );
        mace::rpc::udp::datagram dg(2048);
        datastream<char*> ds(&dg.data.front(),dg.data.size());
        raw::pack( ds, rm );
        mace::cmt::asio::udp::send_to( *m_sock, &dg.data.front(), ds.tellp(), m_ep );
      }

      virtual rpc::message read_message( std::vector<char>&& m ) {
        raw::message rm;
        if( m.size() ) {
            datastream<const char*> ds(&m.front(), m.size() );
            raw::unpack( ds, rm );
        }
        return static_cast<rpc::message>(rm);
      }
  };

} } } } } // mace::rpc::raw::udp::deatil

#endif // _MACE_RPC_RAW_UDP_DETAIL_CONNECTION_HPP
