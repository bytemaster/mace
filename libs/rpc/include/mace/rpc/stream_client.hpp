#ifndef _MACE_RPC_STREAM_CLIENT_HPP_
#define _MACE_RPC_STREAM_CLIENT_HPP_
#include <mace/stub/ptr.hpp>
#include <mace/rpc/client_interface.hpp>

namespace mace { namespace rpc {

  template<typename InterfaceType, typename ConnectionType >
  class stream_client : public mace::stub::ptr<InterfaceType, 
                                               mace::rpc::client_interface< ConnectionType > > 
  {
    public:
      typedef mace::rpc::client_interface< ConnectionType > delegate_type;

      stream_client( const typename ConnectionType::ptr& c )
      :m_con(c){
        delegate_type::set_vtable( *this, m_con );
      }

      template<typename IStream, typename OStream>
      stream_client( IStream& in, OStream& out )
      :m_con( new ConnectionType( in, out ) ){ }

      stream_client( const stream_client& c ):m_con(c.m_con) {
         delegate_type::set_vtable( *this, m_con );
      }

      /**
       *  @return the connection used by this client.
       */
      typename ConnectionType::ptr connection()const { return m_con; }

      void close() {
        if( m_con  ) m_con->close();
      }

    private:
      typename ConnectionType::ptr   m_con;
  };


} } 


#endif
