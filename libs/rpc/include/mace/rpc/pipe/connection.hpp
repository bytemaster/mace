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

        /* This is an impl detail that should not be exposed...
        std::istream& in_stream()  { return static_cast<mace::rpc::pipe::detail::connection*>(my)->in_stream(); }
        std::ostream& out_stream() { return static_cast<mace::rpc::pipe::detail::connection*>(my)->out_stream(); }
        */
      protected:
        connection( mace::rpc::pipe::detail::connection* m )
        :rpc::connection<IODelegate>(m){}

      private:
        connection();
    };
} } } // mace::rpc::pipe
#endif // _MACE_RPC_PIPE_CONNECTION_HPP_
