#ifndef _MACE_RPC_TCP_DETAIL_CONNECTION_HPP_
#define _MACE_RPC_TCP_DETAIL_CONNECTION_HPP_
#include <boost/make_shared.hpp>

#include <mace/rpc/detail/connection_base.hpp>
#include <mace/rpc/raw/message.hpp>
#include <mace/rpc/raw/raw_io.hpp>
#include <mace/rpc/error.hpp>
#include <mace/cmt/bind.hpp>

#include <mace/cmt/asio/tcp/socket.hpp>
#include <mace/cmt/thread.hpp>

namespace mace { namespace rpc { namespace tcp { namespace detail { 

  namespace raw = mace::rpc::raw;

  class connection : public mace::rpc::detail::connection_base {
    public:
      connection(){}

      connection( const boost::asio::ip::tcp::endpoint& ep ) {
        connect( ep );
      }
      connection( const mace::cmt::asio::tcp::socket::ptr& sock )
      :m_sock(sock){
        if( m_sock ) {
          m_read_done = mace::cmt::async( std::bind( &connection::read_loop, this ) );
        }
      }
      void close() {
        if( m_sock ) {
          m_sock->close();
          m_read_done.wait();
          m_sock.reset();
        }
      }
      virtual ~connection() {
        try { close(); } catch ( ... ) { }
      }
      virtual void         send_message( rpc::message&& m ) = 0;
      virtual rpc::message read_message()                   = 0;

      void send( message&& m ) {
        if( m_sock ) {
            send_message( std::move(m) );
        } else {
          MACE_RPC_THROW( "No Connection" );
        }
      }

      void read_loop( ) {
        try {
          while ( true ) {
            //auto m = read_message();
            mace::cmt::async( mace::cmt::bind( [this]( message&& m ) { handle( std::move(m) ); } , read_message() ) );
            //handle( read_message() );
          }
        } catch ( ... ) {
          elog( "connection closed: %1%", 
                boost::current_exception_diagnostic_information() );
        }
        break_promises();
        m_sock.reset();
      }
      void connect( const boost::asio::ip::tcp::endpoint& ep ) {
        close(); 
        try {
            m_sock = std::make_shared<mace::cmt::asio::tcp::socket>();
            m_sock->connect(ep).wait();
            m_read_done = mace::cmt::async( std::bind( &connection::read_loop, this ) );
        } catch ( ... ) {
          m_sock.reset();
          throw;
        }
      }
      void handle_error( message::error_type e, const std::string& msg ) {
        elog( "%1%: %2%", int(e), msg );
      }
      cmt::future<void>                  m_read_done;
      mace::cmt::asio::tcp::socket::ptr m_sock; 
  };


} } } } // mace::rpc::tcp::detail
#endif // _MACE_RPC_TCP_CONNECTION_HPP_
