#ifndef _MACE_RPC_PROCESS_SERVER_HPP_
#define _MACE_RPC_PROCESS_SERVER_HPP_
#include <mace/rpc/server.hpp>
#include <boost/signals.hpp>

namespace mace { namespace rpc { namespace process {

  /**
   *  @tparam ConnectionType must support the mace::rpc::pipe::connection interface.
   */
  template<typename InterfaceType, typename ConnectionType>
  class server : public mace::rpc::server<InterfaceType,ConnectionType> {
    public:
      template<typename SessionType, typename IStream, typename OStream>
      server( const std::shared_ptr<SessionType>& shared_session, IStream& in, OStream& out, const char* read_thread_name = NULL )
      :mace::rpc::server<InterfaceType,ConnectionType>( shared_session ), m_con( new ConnectionType(in,out,read_thread_name) ) { 
        this->sc->init_connection(m_con); 
        m_con->closed.connect( boost::ref(closed) );
      }

      boost::signal<void()> closed;

    private:
      typename ConnectionType::ptr m_con;

  };

} } } // mace::rpc::process

#endif // _MACE_RPC_PROCESS_SERVER_HPP_
