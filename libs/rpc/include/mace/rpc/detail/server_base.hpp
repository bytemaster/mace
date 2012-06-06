#ifndef _MACE_RPC_SERVER_BASE_HPP_
#define _MACE_RPC_SERVER_BASE_HPP_
#include <mace/cmt/thread.hpp>
#include <mace/stub/ptr.hpp>
#include <boost/fusion/support/deduce_sequence.hpp>

namespace mace { namespace rpc { namespace json {
  
  namespace detail {
    class server_base {
      public:

        server_base( session_creator* sc  );
        ~server_base();

      private:
        class tcp_server_base_private* my;
    };
  } // namesapce detal

} } } // mace::rpc::json

#endif // _MACE_RPC_JSON_TCP_SERVER_BASE_HPP_
