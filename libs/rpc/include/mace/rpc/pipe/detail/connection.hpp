#ifndef _MACE_RPC_PIPE_DETAIL_CONNECTION_HPP_
#define _MACE_RPC_PIPE_DETAIL_CONNECTION_HPP_
#include <mace/rpc/detail/connection_base.hpp>
#include <mace/cmt/future.hpp>

namespace mace { 

  namespace rpc { namespace pipe { namespace detail { 

  class connection : public mace::rpc::detail::connection_base {
    public:
      connection( mace::rpc::connection_base& cb, std::istream& in, std::ostream& out );

      ~connection();
      void close();

      virtual void         send_message( rpc::message&& m ) = 0;
      virtual rpc::message read_message()                   = 0;

      /** 
       *  @note kept in header for inlining purposes 
       **/
      void send( message&& m ) {
        send_message( std::move(m) );
      }

      /**
       * Read messages until an exception is thrown.
       *
       * This method may occur in a different thread for 'blocking' input streams.
       * 
       */
      void read_loop( );

      void handle_error( message::error_type e, const std::string& msg );

/*
      std::istream& in_stream()  { return m_in;  }
      std::ostream& out_stream() { return m_out; }
*/

    protected:
      mace::cmt::thread*      m_read_thread;
      cmt::future<void>       m_read_done;

      std::istream&           m_in;
      std::ostream&           m_out;
      mace::cmt::thread&      m_created_thread;
  };


} } } } // mace::rpc::pipe::detail
#endif // _MACE_RPC_TCP_CONNECTION_HPP_
