#ifndef _MACE_RPC_UDP_SERVER_IPP_
#define _MACE_RPC_UDP_SERVER_IPP_
#include <mace/rpc/udp/server.hpp>
#include <mace/cmt/bind.hpp>

namespace mace { namespace rpc { namespace udp {

  template<typename InterfaceType, typename ConnectionType>
  template<typename SessionType>
  server<InterfaceType,ConnectionType>::server( const boost::function<std::shared_ptr<SessionType>()>& sg, const endpoint& port )
  :mace::rpc::server<InterfaceType,ConnectionType>( sg ) {
    listen_complete = cmt::async( [=]{this->listen(port);} ); 
  }

  template<typename InterfaceType, typename ConnectionType>
  template<typename SessionType>
  server<InterfaceType,ConnectionType>::server( const std::shared_ptr<SessionType>& shared_session, const endpoint& port )
  :mace::rpc::server<InterfaceType,ConnectionType>( shared_session ) {
    listen_complete = cmt::async( [=]{ this->listen(port); } ); 
  }
  template<typename InterfaceType, typename ConnectionType>
  server<InterfaceType,ConnectionType>::~server() {
    try {
      if( sock ) sock->close();
      listen_complete.wait();
    }catch(...){
      elog( "%1%", boost::current_exception_diagnostic_information() );
    }
  }
  template<typename InterfaceType, typename ConnectionType>
  void server<InterfaceType,ConnectionType>::listen( const endpoint& p ) {
    try {
      sock = std::make_shared<boost::asio::ip::udp::socket>( std::ref(mace::cmt::asio::default_io_service()), p );
      do {
        datagram dg(2048);
        size_t s = mace::cmt::asio::udp::receive_from( *sock, &dg.data.front(), dg.data.size(), dg.ep );
        dg.data.resize(s);

        auto i = ep_to_con.find(dg.ep);
        if( i == ep_to_con.end() ) {
           // New Connection
           auto s = std::make_shared<socket_t>( std::ref( mace::cmt::asio::default_io_service() ) );
           s->open(boost::asio::ip::udp::v4());
           auto p = std::make_shared<connection_type>(s);
           ep_to_con[dg.ep] = p;
           on_connection(p);
           cmt::async( [=]{ this->on_connection(p); } );
           cmt::async( cmt::bind( std::bind(&ConnectionType::handle_datagram,p,std::placeholders::_1), std::move(dg) ) );
        } else {
          i->second->handle_datagram( std::move(dg) );
        }
      }while( true );
    } catch (...) {
      elog( "%1%", boost::current_exception_diagnostic_information() );
    }
  }
  template<typename InterfaceType, typename ConnectionType>
  void server<InterfaceType,ConnectionType>::on_connection( const typename ConnectionType::ptr& c ) {
    c->closed.connect( [=]{this->on_disconnect(c);}  );
    connections[c] = this->sc->init_connection(c);
  }
  template<typename InterfaceType, typename ConnectionType>
  void server<InterfaceType,ConnectionType>::on_disconnect( const typename ConnectionType::ptr& c ) {
    connections.erase( c );
  }

} } }

#endif // _MACE_RPC_UDP_SERVER_IPP_
