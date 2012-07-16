#ifndef _MACE_RPC_JSON_PROCSS_CLIENT_HPP_
#define _MACE_RPC_JSON_PROCSS_CLIENT_HPP_
#include <mace/stub/ptr.hpp>
#include <mace/rpc/process/client.hpp>
#include <mace/rpc/json/json_stream_connection.hpp>

namespace mace { namespace rpc {  namespace json { namespace process {

  typedef mace::rpc::json::stream_connection<mace::cmt::process::istream,mace::cmt::process::ostream> connection;

#ifndef BOOST_NO_TEMPLATE_ALIASES
  template<typename Interface>
  using client = mace::rpc::proces::client<Interface,connection>;
#else // BOOST_NO_TEMPLATE_ALIASES 

  /**
   *   Given a connection, map methods on InterfaceType into RPC calls
   *   with the expected results returned as futures as defined by rpc::client_interface
   *
   *   @todo - make client_interface take a pointer to client_base to enable the
   *          construction of one client<Interface> and then quickly change the 
   *          connection object without having to iterate over the methods again.
   */
  template<typename InterfaceType>
  class client : public mace::rpc::process::client<InterfaceType,connection> {
    public:
      typedef std::shared_ptr<client>                       ptr;
      typedef mace::rpc::client_interface< connection > delegate_type;

      client(){}

      client( const client& c )
      :mace::rpc::process::client<InterfaceType,connection>(c){}

      client( client&& c )
      :mace::rpc::process::client<InterfaceType,connection>(std::move(c)){}

      client( const typename connection::ptr& c) 
      :mace::rpc::process::client<InterfaceType,connection>(c){}

      using mace::rpc::process::client<InterfaceType,connection>::operator!;
      using mace::rpc::process::client<InterfaceType,connection>::operator=;

  };
#endif // BOOST_NO_TEMPLATE_ALIASES

} } } } // mace::rpc::json::process

#endif // _MACE_RPC_JSON_PROCSS_CLIENT_HPP_
