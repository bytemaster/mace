#ifndef _MACE_RPC_TCP_CONNECTION_HPP_
#define _MACE_RPC_TCP_CONNECTION_HPP_
#include <mace/rpc/connection.hpp>
#include <mace/cmt/asio/tcp/socket.hpp>
#include <mace/rpc/tcp/detail/connection.hpp>

namespace mace { namespace rpc { namespace tcp {
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

        void connect( const boost::asio::ip::tcp::endpoint& ep ) {
          slog( "this: %1%  my: %2%", this, this->my );
          static_cast<mace::rpc::tcp::detail::connection*>(this->my)->connect(ep);
        }

        void send( message&& m ){
          slog( "this: %1%  my: %2%", this, this->my );
          this->my->send( std::move(m) );
        }
      protected:
        connection( mace::rpc::tcp::detail::connection* m )
        :rpc::connection<IODelegate>(m){}

      private:
        connection();
    };
} } } // mace::rpc::tcp
#endif // _MACE_RPC_TCP_CONNECTION_HPP_
