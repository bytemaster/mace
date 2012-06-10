#ifndef _MACE_RPC_CLIENT_HPP_
#define _MACE_RPC_CLIENT_HPP_
#include <mace/stub/ptr.hpp>
#include <mace/rpc/client_interface.hpp>
#include <mace/rpc/raw/tcp/connection.hpp>

namespace mace { namespace rpc {  

  /**
   *   Given a connection, map methods on InterfaceType into RPC calls
   *   with the expected results returned as futures as defined by rpc::client_interface
   *
   *   @todo - make client_interface take a pointer to client_base to enable the
   *          construction of one client<Interface> and then quickly change the 
   *          connection object without having to iterate over the methods again.
   */
  template<typename InterfaceType, typename ConnectionType = mace::rpc::raw::tcp::connection<raw_io> >
  class client : public mace::stub::ptr<InterfaceType, mace::rpc::client_interface< ConnectionType > > {
    public:
      typedef std::shared_ptr<client>                       ptr;
      typedef mace::rpc::client_interface< ConnectionType > delegate_type;

      client(){}

      client( const client& c ):m_con(c.m_con) {
         delegate_type::set_vtable( *this, m_con );
      }

      client( client&& c ) {
        m_con = std::move(c.m_con);
      }

      bool operator!()const { return !m_con; }

      client& operator=( const client& c ) {
        if( &c != this )  {
            m_con = c.m_con;
            delegate_type::set_vtable( *this, m_con );
        }
        return *this;
      }

      client( const typename ConnectionType::ptr& c) 
      :m_con(c) {
        delegate_type::set_vtable( *this, m_con );
      }      

      template<typename T>
      void connect( T&& v ) {
        typename ConnectionType::ptr c( new ConnectionType( std::forward<T>(v) ) );
        m_con = c;
        delegate_type::set_vtable( *this, m_con );
      }

      template<typename T, typename T2>
      void connect( T&& v, T2&& v2 ) {
        typename ConnectionType::ptr c( new ConnectionType( std::forward<T>(v), std::forward<T>(v2)  ) );
        m_con = c;
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

} } // mace::rpc::json

#endif
