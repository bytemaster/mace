#ifndef _MACE_RPC_JSON_STREAM_CLIENT_HPP_
#define _MACE_RPC_JSON_STREAM_CLIENT_HPP_
#include <mace/stub/ptr.hpp>
#include <mace/rpc/client_interface.hpp>
#include <mace/rpc/json/pipe/connection.hpp>

namespace mace { namespace rpc {  namespace json { 

  /**
   *   Given a connection, map methods on InterfaceType into RPC calls
   *   with the expected results returned as futures as defined by rpc::client_interface
   *
   *   @todo - make client_interface take a pointer to client_base to enable the
   *          construction of one client<Interface> and then quickly change the 
   *          connection object without having to iterate over the methods again.
   */
  template<typename InterfaceType, typename IStream=std::istream, typename OStream = std::ostream>
  class stream_client : public mace::rpc::stream_client<InterfaceType,
                                 mace::rpc::client_interface< mace::rpc::json::stream_connection<IStream,OStream> > > 
  {
    public:
      typedef mace::rpc::stream_client<InterfaceType,mace::rpc::client_interface<connection_type> > base_type;
      typedef mace::rpc::client_interface< ConnectionType > delegate_type;

      client(){}

      client( const client& c ):base_type(c){}

      client( client&& c ):base_type(std::move(c)){}

      client( const typename connection_type::ptr& c) 
      :m_con(c) { delegate_type::set_vtable( *this, m_con ); }      

      using base_type::operator=;

      void connect( IStream& in, OStream& out) {
        set_connection( new connection_type( in, out ) );
      }

      /*
      client& operator=( const client& c ) {
        return *((base_type*)this) = c;
      }
      client& operator=( client&& c ) {
        return *((base_type*)this) = std::move(c);
        
        mace::stub::ptr<InterfaceType,delegate_type>& base = *this;
        std::swap( m_con, c.m_con );
        base = std::move(c);
        return *this;
      }
      */
  };

} } }  // mace::rpc::json

#endif // _MACE_RPC_JSON_TCP_CLIENT_HPP_
