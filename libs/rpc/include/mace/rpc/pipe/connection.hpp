#ifndef _MACE_RPC_PIPE_CONNECTION_HPP_
#define _MACE_RPC_PIPE_CONNECTION_HPP_
#include <mace/rpc/connection.hpp>
#include <mace/rpc/pipe/detail/connection.hpp>

namespace mace { namespace rpc { namespace pipe {
    namespace detail { class connection; }

    /**
     *  Handles stream details, but leaves the read loop and
     *  the send method 
     */
    template<typename IODelegate> 
    class connection : public mace::rpc::connection<IODelegate> {
      public:
        typedef std::shared_ptr<connection> ptr;

        void send( message&& m ){ this->my->send( std::move(m) ); }

      protected:
        connection( mace::rpc::pipe::detail::connection* m )
        :rpc::connection<IODelegate>(m){}

      private:
        connection();
    };
} } } // mace::rpc::pipe
#endif // _MACE_RPC_PIPE_CONNECTION_HPP_
