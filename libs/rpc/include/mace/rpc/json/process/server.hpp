#ifndef  MACE_RPC_JSON_PROCESS_SERVER_HPP
#define  MACE_RPC_JSON_PROCESS_SERVER_HPP
#include <mace/rpc/process/server.hpp>
#include <mace/rpc/json/pipe/connection.hpp>

namespace mace { namespace rpc {  namespace json { namespace process {

#ifndef BOOST_NO_TEMPLATE_ALIASES
  template<typename Interface, typename IODelegate=json::io>
  using server = mace::rpc::server<Interface,mace::rpc::json::pipe::connection<IODelegate> >;
#else // BOOST_NO_TEMPLATE_ALIASES 

  template<typename Interface, typename IODelegate=mace::rpc::json::io>
  class server : public mace::rpc::process::server<Interface,mace::rpc::json::pipe::connection<IODelegate> > {
    public:
      typedef mace::rpc::json::pipe::connection<IODelegate> connection_type;

      template<typename SessionType>
      server( const boost::function<std::shared_ptr<SessionType>()>& sg, std::istream& i, std::ostream& o )
      :mace::rpc::process::server<Interface,connection_type>( sg, i, o ){}

      template<typename SessionType>
      server( const std::shared_ptr<SessionType>& shared_session,  std::istream& i, std::ostream& o )
      :mace::rpc::process::server<Interface,connection_type>( shared_session, i, o ){}

      template<typename SessionType>
      server( SessionType* shared_session,  std::istream& i, std::ostream& o )
      :mace::rpc::process::server<Interface,connection_type>( shared_session, i, o ){}
  };
#endif


} } } } // mace::rpc::json::process
#endif // MACE_RPC_JSON_PROCESS_SERVER_HPP
