#ifndef _MACE_RPC_JSON_PIPE_CONNECTION_HPP_
#define _MACE_RPC_JSON_PIPE_CONNECTION_HPP_
#include <mace/rpc/json/pipe/detail/connection.hpp>
#include <mace/rpc/pipe/connection.hpp>
#include <mace/rpc/json/io.hpp>

namespace mace { namespace rpc { namespace json { namespace pipe {

    template<typename IODelegate = json::io>
    class connection : public mace::rpc::pipe::connection<IODelegate> {
      public:
        typedef std::shared_ptr<connection> ptr;

        connection( std::istream& in, std::ostream& out) 
        :rpc::pipe::connection<IODelegate>( new mace::rpc::json::pipe::detail::connection(in,out) ) {}
    };

} } } } // mace::rpc::json::pipe

#endif // _MACE_RPC_RAW_PIPE_CONNECTION_HPP_
