#ifndef _MACE_RPC_PIPE_DETAIL_CONNECTION_HPP_
#define _MACE_RPC_PIPE_DETAIL_CONNECTION_HPP_
#include <boost/make_shared.hpp>
#include <mace/rpc/detail/connection_base.hpp>
#include <mace/rpc/raw/message.hpp>
#include <mace/rpc/raw/raw_io.hpp>
#include <mace/rpc/error.hpp>
#include <mace/cmt/bind.hpp>
#include <mace/cmt/thread.hpp>

namespace mace { namespace rpc { namespace pipe { namespace detail { 

  namespace raw = mace::rpc::raw;

  class connection : public mace::rpc::detail::connection_base {
    public:
      connection( std::istream& in, std::ostream& out )
      :m_in(in),m_out(out),m_created_thread(mace::cmt::thread::current()) {
         m_read_thread = mace::cmt::thread::create("read_loop");
         m_read_done = m_read_thread->async( [this](){read_loop();} );
      }

      ~connection() {
         m_read_thread->quit();
      }

      virtual void         send_message( rpc::message&& m ) = 0;
      virtual rpc::message read_message()                   = 0;

      void send( message&& m ) {
        send_message( std::move(m) );
      }
      /**
       * Occurs in the read_loop thread because reading from
       * streams is 'blocking'.
       */
      void read_loop( ) {
        try {
          while ( true ) {
            //auto m = read_message();
            m_created_thread.async( mace::cmt::bind( [this]( message&& m ) { handle( std::move(m) ); } , read_message() ) );
            //handle( read_message() );
          }
        } catch ( ... ) {
          elog( "connection closed: %1%", 
                boost::current_exception_diagnostic_information() );
        }
        break_promises();
      }

      void handle_error( message::error_type e, const std::string& msg ) {
        elog( "%1%: %2%", int(e), msg );
      }
      mace::cmt::thread*                 m_read_thread;
      cmt::future<void>                  m_read_done;

      std::istream&           m_in;
      std::ostream&           m_out;
      mace::cmt::thread&      m_created_thread;
  };


} } } } // mace::rpc::pipe::detail
#endif // _MACE_RPC_TCP_CONNECTION_HPP_
