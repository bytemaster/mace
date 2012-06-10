#ifndef _MACE_RPC_UDP_CONNECTION_HPP_
#define _MACE_RPC_UDP_CONNECTION_HPP_
#include <mace/rpc/connection.hpp>
#include <mace/cmt/asio/udp/socket.hpp>
#include <mace/rpc/udp/detail/connection.hpp>

namespace mace { namespace rpc { namespace udp {
    namespace detail { class connection; }

    /**
     *  Handles TCP socket details, but leaves the read loop and
     *  the send method 
     */
    template<typename IODelegate> 
    class connection : public mace::rpc::connection<IODelegate> {
      public:
        typedef std::shared_ptr<connection> ptr;

        void close() { this->my->close(); }

        void connect( const boost::asio::ip::udp::endpoint& ep ) {
          static_cast<mace::rpc::udp::detail::connection*>(this->my)->connect(ep);
        }

        void send( message&& m ){
          this->my->send( std::move(m) );
        }

        void handle_datagram( datagram&& dg ) {
          static_cast<mace::rpc::udp::detail::connection*>(this->my)->handle_datagram( std::move(dg) );
        }
      protected:
        connection( mace::rpc::udp::detail::connection* m )
        :rpc::connection<IODelegate>(m){}

      private:
        connection();
    };
} } } // mace::rpc::udp
#endif // _MACE_RPC_TCP_CONNECTION_HPP_
