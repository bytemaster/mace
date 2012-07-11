#ifndef _MACE_RPC_RAW_UDP_CONNECTION_HPP_
#define _MACE_RPC_RAW_UDP_CONNECTION_HPP_
#include <mace/rpc/raw/udp/detail/connection.hpp>
#include <mace/rpc/udp/connection.hpp>

namespace mace { namespace rpc { namespace raw { namespace udp {

    template<typename IODelegate>
    class connection : public mace::rpc::udp::connection<IODelegate> {
      public:
        typedef std::shared_ptr<connection> ptr;

        connection()
        :rpc::udp::connection<IODelegate>( new mace::rpc::raw::udp::detail::connection(*this) ) {}

        connection( const boost::asio::ip::udp::endpoint& ep )
        :rpc::udp::connection<IODelegate>( new mace::rpc::raw::udp::detail::connection(*this,ep) ){}

        connection( const mace::rpc::udp::socket_ptr& sock )
        :rpc::udp::connection<IODelegate>( new mace::rpc::raw::udp::detail::connection(*this,sock) ) {}
    };

} } } } // mace::rpc::raw::udp

#endif // _MACE_RPC_RAW_UDP_CONNECTION_HPP_
