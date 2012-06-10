#ifndef _MACE_RPC_RAW_TCP_CONNECTION_HPP_
#define _MACE_RPC_RAW_TCP_CONNECTION_HPP_
#include <mace/rpc/raw/tcp/detail/connection.hpp>
#include <mace/rpc/tcp/connection.hpp>

namespace mace { namespace rpc { namespace raw { namespace tcp {

    template<typename IODelegate>
    class connection : public mace::rpc::tcp::connection<IODelegate> {
      public:
        typedef std::shared_ptr<connection> ptr;

        connection()
        :rpc::tcp::connection<IODelegate>( new mace::rpc::raw::tcp::detail::connection() ) {}

        connection( const boost::asio::ip::tcp::endpoint& ep )
        :rpc::tcp::connection<IODelegate>( new mace::rpc::raw::tcp::detail::connection(ep) ){}

        connection( const mace::cmt::asio::tcp::socket::ptr& sock )
        :rpc::tcp::connection<IODelegate>( new mace::rpc::raw::tcp::detail::connection(sock) ) {}
    };

} } } } // mace::rpc::raw::tcp

#endif // _MACE_RPC_RAW_TCP_CONNECTION_HPP_
