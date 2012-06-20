#ifndef _MACE_RPC_TCP_SERVER_HPP_
#define _MACE_RPC_TCP_SERVER_HPP_
#include <mace/rpc/server.hpp>
#include <mace/cmt/asio.hpp>
#include <mace/cmt/asio/tcp/socket.hpp>

namespace mace { namespace rpc { namespace tcp {

  template<typename InterfaceType, typename ConnectionType>
  class server : public mace::rpc::server<InterfaceType,ConnectionType> {
    public:
      typedef ConnectionType            connection_type;

      template<typename SessionType>
      server( const boost::function<std::shared_ptr<SessionType>()>& sg, uint16_t port )
      :mace::rpc::server<InterfaceType,ConnectionType>( sg ) {
        listen_complete = cmt::async( [=](){ this->listen(port); } );
      }

      template<typename SessionType>
      server( const std::shared_ptr<SessionType>& shared_session, uint16_t port )
      :mace::rpc::server<InterfaceType,ConnectionType>( shared_session ) {
        listen_complete = cmt::async( [=](){ this->listen(port); } );
      }

      ~server() {
        try {
          if( acc ) acc->close();
          listen_complete.wait();
        }catch(...){
          elog( "%1%", boost::current_exception_diagnostic_information() );
        }
      }

    private:
	    typedef cmt::asio::tcp::socket socket_t;

      void on_connection( const typename ConnectionType::ptr& c ) {
        c->closed.connect( boost::bind( &server::on_disconnect, this, c ) );
        connections[c] = this->sc->init_connection(c);
      }
      void on_disconnect( const typename ConnectionType::ptr& c ) {
        connections.erase( c );
      }

      void listen( uint16_t p ) {
        slog( "listen! %1%", p );
        try {
          acc = boost::make_shared<boost::asio::ip::tcp::acceptor>( 
                    boost::ref(mace::cmt::asio::default_io_service()),
                    boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(),p) );
          boost::system::error_code ec;
          do {
              socket_t::ptr iosp(new socket_t());
              ec = cmt::asio::tcp::accept( *acc, *iosp);
              if(!ec) {
                  cmt::async( boost::bind(&server::on_connection, this,
                              typename ConnectionType::ptr( new ConnectionType(iosp) ) )); 
              } else { 
                  elog( "%1%", boost::system::system_error(ec).what() );
              }
          }while( !ec );
          
        } catch (...) {
          elog( "%1%", boost::current_exception_diagnostic_information() );
        }
      }

      boost::shared_ptr<boost::asio::ip::tcp::acceptor>  acc;
      mace::cmt::future<void>                            listen_complete;
      std::map<typename ConnectionType::ptr,boost::any>  connections;
  };

} } } // mace::rpc::tcp

#endif
