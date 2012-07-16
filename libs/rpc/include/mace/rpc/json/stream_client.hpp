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
  class stream_client : public mace::stub::ptr<InterfaceType, 
                               mace::rpc::client_interface< mace::rpc::json::stream_connection<IStream,OStream> > > 
  {
    public:
      typedef mace::rpc::json::stream_connection<IStream,OStream> > connection_type;
      typedef std::shared_ptr<client>                       ptr;
      typedef mace::rpc::client_interface< ConnectionType > delegate_type;

      client(){}

      client( const client& c ):m_con(c.m_con) {
         delegate_type::set_vtable( *this, m_con );
      }

      client( client&& c ) { m_con = std::move(c.m_con); }

      bool operator!()const { return !m_con; }

      client& operator=( const client& c ) {
        if( &c != this )  {
            m_con = c.m_con;
            delegate_type::set_vtable( *this, m_con );
        }
        return *this;
      }
      client& operator=( client&& c ) {
        mace::stub::ptr<InterfaceType,delegate_type>& base = *this;
        std::swap( m_con, c.m_con );
        base = std::move(c);
        return *this;
      }

      client( const typename connection_type::ptr& c) 
      :m_con(c) { delegate_type::set_vtable( *this, m_con ); }      

      void connect( IStream& in, OStream& out) {
        m_con.reset( new connection_type( in, out ) );
        delegate_type::set_vtable( *this, m_con );
      }

      typename connection_type::ptr connection()const { return m_con; }

      void set_connection( const typename connection_type::ptr& c ) {
        m_con = c;
        delegate_type::set_vtable( *this, m_con );
      }
    private:
      typename connection_type::ptr m_con;
  };

} } }  // mace::rpc::json

#endif // _MACE_RPC_JSON_TCP_CLIENT_HPP_
