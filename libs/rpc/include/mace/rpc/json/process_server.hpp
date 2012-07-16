#ifndef  MACE_RPC_JSON_PROCESS_SERVER_HPP
#define  MACE_RPC_JSON_PROCESS_SERVER_HPP
#include <mace/rpc/process/server.hpp>
#include <mace/rpc/json/json_stream_connection.hpp>

namespace mace { namespace rpc {  namespace json { namespace process {

  typedef mace::rpc::json::stream_connection<std::istream,std::ostream> server_connection;
#ifndef BOOST_NO_TEMPLATE_ALIASES
  template<typename Interface>
  using server = mace::rpc::server<Interface,server_connection>;
#else // BOOST_NO_TEMPLATE_ALIASES 

  template<typename Interface>
  class server : public mace::rpc::process::server<Interface,server_connection> {
    public:
      typedef server_connection connection_type;

      template<typename SessionType>
      server( const boost::function<std::shared_ptr<SessionType>()>& sg, std::istream& i, std::ostream& o, const char* read_thread_name = NULL )
      :mace::rpc::process::server<Interface,connection_type>( sg, i, o, read_thread_name ){}

      template<typename SessionType>
      server( const std::shared_ptr<SessionType>& shared_session,  std::istream& i, std::ostream& o, const char* read_thread_name = NULL )
      :mace::rpc::process::server<Interface,connection_type>( shared_session, i, o, read_thread_name ){}

      template<typename SessionType>
      server( SessionType* shared_session,  std::istream& i, std::ostream& o, const char* read_thread_name = NULL )
      :mace::rpc::process::server<Interface,connection_type>( shared_session, i, o, read_thread_name ){}
  };
#endif


} } } } // mace::rpc::json::process
#endif // MACE_RPC_JSON_PROCESS_SERVER_HPP
