#ifndef _MACE_RPC_JSON_TCP_CLIENT_HPP_
#define _MACE_RPC_JSON_TCP_CLIENT_HPP_
#include <mace/stub/ptr.hpp>
#include <mace/rpc/client_interface.hpp>
#include <mace/rpc/json/pipe/connection.hpp>

namespace mace { namespace rpc {  namespace json { namespace pipe {

  /**
   *   Given a connection, map methods on InterfaceType into RPC calls
   *   with the expected results returned as futures as defined by rpc::client_interface
   *
   *   @todo - make client_interface take a pointer to client_base to enable the
   *          construction of one client<Interface> and then quickly change the 
   *          connection object without having to iterate over the methods again.
   */
  template<typename InterfaceType, typename ConnectionType = mace::rpc::json::pipe::connection<json::io> >
  class client : public mace::stub::ptr<InterfaceType, mace::rpc::client_interface< ConnectionType > > {
    public:
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
        std::swap( *this, c );
        return *this;
      }

      client( const typename ConnectionType::ptr& c) 
      :m_con(c) {
        delegate_type::set_vtable( *this, m_con );
      }      

      void connect( std::istream& in, std::ostream& out) {
        m_con.reset( new ConnectionType( in, out ) );
        delegate_type::set_vtable( *this, m_con );
      }

      typename ConnectionType::ptr connection()const { return m_con; }

      void set_connection( const typename ConnectionType::ptr& c ) {
        m_con = c;
        delegate_type::set_vtable( *this, m_con );
      }
    private:
      typename ConnectionType::ptr m_con;
  };

} } } } // mace::rpc::json

#endif // _MACE_RPC_JSON_TCP_CLIENT_HPP_
