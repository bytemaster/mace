#ifndef _MACE_RPC_UDP_DETAIL_CONNECTION_HPP_
#define _MACE_RPC_UDP_DETAIL_CONNECTION_HPP_
#include <boost/make_shared.hpp>
#include <mace/rpc/detail/connection_base.hpp>
#include <mace/rpc/raw/message.hpp>
#include <mace/rpc/raw/raw_io.hpp>
#include <mace/rpc/error.hpp>
#include <mace/rpc/udp/datagram.hpp>

#include <mace/cmt/asio/udp/socket.hpp>
#include <mace/cmt/asio.hpp>
#include <mace/cmt/thread.hpp>

#include <boost/asio.hpp>

namespace mace { namespace rpc { namespace udp { 

	typedef boost::asio::ip::udp::socket socket_t;
  typedef std::shared_ptr<socket_t>  socket_ptr;

  namespace detail { 

  namespace raw = mace::rpc::raw;

  /**
   *  TODO: move implementation to CPP
   */
  class connection : public mace::rpc::detail::connection_base {
    public:
      connection(mace::rpc::connection_base& cb)
      :mace::rpc::detail::connection_base(cb){}

      connection( mace::rpc::connection_base& cb, const boost::asio::ip::udp::endpoint& ep )
      :mace::rpc::detail::connection_base(cb) {
        connect( ep );
      }
      connection( mace::rpc::connection_base& cb, const socket_ptr& sock )
      :mace::rpc::detail::connection_base(cb), m_sock(sock){
        if( m_sock ) {
          m_read_done = mace::cmt::async( [=]{ this->read_loop(); } );
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
      virtual rpc::message read_message( std::vector<char>&& m ) = 0;

      void send( message&& m ) {
        if( m_sock ) {
            send_message( std::move(m) );
        } else {
          MACE_RPC_THROW( "No Connection" );
        }
      }

      void handle_datagram( datagram&& dg ) {
        m_ep = dg.ep;
        handle( read_message( std::move(dg.data) ) );
      }

      datagram read_datagram() {
        datagram dg(2048);
        size_t s = mace::cmt::asio::udp::receive_from(*m_sock,
                                            &dg.data.front(),
                                            dg.data.size(),dg.ep);
        dg.data.resize(s);
        return dg;
      }

      void read_loop( ) {
        try {
          while ( true ) {
             handle_datagram( read_datagram() );
          }
        } catch ( ... ) {
          elog( "connection closed: %1%", 
                boost::current_exception_diagnostic_information() );
        }
        break_promises();
        m_sock.reset();
      }
      void connect( const boost::asio::ip::udp::endpoint& ep ) {
        close(); 
        try {
          m_sock = std::make_shared<socket_t>( std::ref(mace::cmt::asio::default_io_service()) );
          m_sock->open(boost::asio::ip::udp::v4());
          m_ep = ep;
          m_read_done = mace::cmt::async( [=]{ this->read_loop(); } );
        } catch ( ... ) {
          m_sock.reset();
          throw;
        }
      }
      void handle_error( message::error_type e, const std::string& msg ) {
        elog( "%1%: %2%", int(e), msg );
      }
      cmt::future<void>                  m_read_done;
      boost::asio::ip::udp::endpoint     m_ep;
      socket_ptr                         m_sock;
  };


} } } } // mace::rpc::udp::detail
#endif // _MACE_RPC_UDP_CONNECTION_HPP_
