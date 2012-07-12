#ifndef _MACE_RPC_JSON_PROCSS_CLIENT_HPP_
#define _MACE_RPC_JSON_PROCSS_CLIENT_HPP_
#include <mace/stub/ptr.hpp>
#include <mace/rpc/process/client.hpp>
#include <mace/rpc/json/pipe/connection.hpp>

namespace mace { namespace rpc {  namespace json { namespace process {

#ifndef BOOST_NO_TEMPLATE_ALIASES
  template<typename Interface, typename IODelegate=json::io>
  using client = mace::rpc::proces::client<Interface,mace::rpc::json::pipe::connection<IODelegate> >;
#else // BOOST_NO_TEMPLATE_ALIASES 

  /**
   *   Given a connection, map methods on InterfaceType into RPC calls
   *   with the expected results returned as futures as defined by rpc::client_interface
   *
   *   @todo - make client_interface take a pointer to client_base to enable the
   *          construction of one client<Interface> and then quickly change the 
   *          connection object without having to iterate over the methods again.
   */
  template<typename InterfaceType, typename ConnectionType = mace::rpc::json::pipe::connection<json::io> >
  class client : public mace::rpc::process::client<InterfaceType,ConnectionType> {
    public:
      typedef std::shared_ptr<client>                       ptr;
      typedef mace::rpc::client_interface< ConnectionType > delegate_type;

      client(){}

      client( const client& c )
      :mace::rpc::process::client<InterfaceType,ConnectionType>(c){}

      client( client&& c )
      :mace::rpc::process::client<InterfaceType,ConnectionType>(std::move(c)){}

      client( const typename ConnectionType::ptr& c) 
      :mace::rpc::process::client<InterfaceType,ConnectionType>(c){}

      using mace::rpc::process::client<InterfaceType,ConnectionType>::operator!;
      using mace::rpc::process::client<InterfaceType,ConnectionType>::operator=;

  };
#endif // BOOST_NO_TEMPLATE_ALIASES

} } } } // mace::rpc::json

#endif // _MACE_RPC_JSON_PROCSS_CLIENT_HPP_
