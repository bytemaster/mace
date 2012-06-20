#ifndef _MACE_RPC_JSON_TCP_SERVER_HPP_
#define _MACE_RPC_JSON_TCP_SERVER_HPP_
#include <mace/rpc/tcp/server.hpp>
#include <mace/rpc/json/io.hpp>
#include <mace/rpc/json/tcp/connection.hpp>

namespace mace { namespace rpc { namespace json { namespace tcp {

#ifndef BOOST_NO_TEMPLATE_ALIASES
  template<typename Interface, typename IODelegate=json::io>
  using server = mace::rpc::tcp::server<Interface,mace::rpc::json::tcp::connection<IODelegate> >;
#else // BOOST_NO_TEMPLATE_ALIASES 

  /**
   *  A TCP server that accepts connections using the json rpc message
   *  transport and using IODelegate to pass parameters/results.  Defaults to json::io
   *  for parameters and results.
   *
   *  This is really just a work around for lack of template alias's in VC++
   *
   *  @note this is a workaround for lack of template alias support on anything but g++ 4.7
   */
  template<typename Interface, typename IODelegate=mace::rpc::json::io>
  class server : public mace::rpc::tcp::server<Interface,mace::rpc::json::tcp::connection<IODelegate> > {
    public:
      typedef mace::rpc::json::tcp::connection<IODelegate> connection_type;

      template<typename SessionType>
      server( const boost::function<std::shared_ptr<SessionType>()>& sg, uint16_t port )
      :mace::rpc::tcp::server<Interface,connection_type>( sg, port ){}

      template<typename SessionType>
      server( const std::shared_ptr<SessionType>& shared_session, uint16_t port )
      :mace::rpc::tcp::server<Interface,connection_type>( shared_session, port ){}

      template<typename SessionType>
      server( SessionType* shared_session, uint16_t port )
      :mace::rpc::tcp::server<Interface,connection_type>( shared_session, port ){}
  };
#endif

} } } } // mace::rpc::json::tcp

#endif // _MACE_RPC_JSON_TCP_SERVER_HPP_
