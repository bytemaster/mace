#include <mace/ssh/process.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/flush.hpp>
#include <boost/iostreams/stream.hpp>
#include "client_detail.hpp"



namespace mace { namespace ssh { 

  namespace detail {

    namespace io = boost::iostreams;


    class process_source : public io::source {
      public:
        typedef char      type;

        process_source( process_d& p, int chan )
        :m_process(p),m_chan(chan){}

        std::streamsize read( char* s, std::streamsize n );

      private:
        process_d&      m_process;
        int             m_chan;
    };
    /*
    class process_sink : public io::sink {
      public:
        struct category : io::sink::category, io::flushable_tag {};
        typedef char      type;

        process_sink( process_d& p, int chan )
        :m_process(p),m_chan(chan){}

        std::streamsize write( const char* s, std::streamsize n );
        void close();
        bool flush();

      private:
        process_d&      m_process;
        int             m_chan;
    };
    */




    struct process_d {
      std::string                           command;
      std::shared_ptr<mace::ssh::client>    sshc;
      mace::cmt::promise<int>::ptr          result;
      LIBSSH2_CHANNEL*                      chan;

      io::stream<process_source>            std_out;
      io::stream<process_source>            std_err;
      io::stream<process_sink>              std_in;

      int read_some( char* data, size_t len, int stream_id );
      int write_some( const char* data, size_t len, int stream_id );

      /**
       *  @pre c is connected and has a valid session
       */
      process_d( mace::ssh::client& c, const std::string& cmd )
      :sshc(c.shared_from_this()),
       std_out(process_source(*this,0)),
       std_err(process_source(*this,1)),
       std_in(process_sink(*this,0) ) 
       {
        BOOST_ASSERT( c.my->m_session );

        chan = c.my->open_channel(); 

        int ec = libssh2_channel_exec( chan, cmd.c_str() );
        while( ec == LIBSSH2_ERROR_EAGAIN ) {
          sshc->my->wait_on_socket();
          ec = libssh2_channel_exec( chan, cmd.c_str() );
        }

        if( ec ) {
           char* msg = 0;
           ec   = libssh2_session_last_error( sshc->my->m_session, &msg, 0, 0 );
           MACE_SSH_THROW( "libssh2_channel_exec failed: %1% - %2%", %ec %msg  );
        }
      }

      void send_eof() {

        int ec = libssh2_channel_send_eof( chan );
        while( ec == LIBSSH2_ERROR_EAGAIN ) {
          sshc->my->wait_on_socket();
          ec = libssh2_channel_send_eof( chan );
        }
        if( ec ) {
        }
      }
      bool flush(int stream_id) {

        int ec = libssh2_channel_flush_ex( chan, stream_id );
        while( ec == LIBSSH2_ERROR_EAGAIN ) {
          sshc->my->wait_on_socket();
          ec = libssh2_channel_flush_ex( chan, stream_id );
        }
        if( ec ) {
          char* msg = 0;
          ec = libssh2_session_last_error( sshc->my->m_session, &msg, 0, 0 );
          MACE_SSH_THROW( "flush failed: %1% - %2%", %ec %msg  );
        }
        return true;
      }
    };

    int process_d::write_some( const char* data, size_t len, int stream_id ) {
       int rc;
       const char* buf = data;
       size_t buflen = len;
       do {
           rc = libssh2_channel_write_ex( chan, stream_id, buf, buflen );
           if( rc > 0 ) {
              buf += rc;
              buflen -= rc;
              return buf-data;
           } else if( rc == 0 ) {
              if( libssh2_channel_eof( chan ) )  {
                return -1; // eof
              }
           } else {
  
             if( rc == LIBSSH2_ERROR_EAGAIN ) {
               if( 0 < (buf-data) ) {
                 return buf-data;
               }
               else  {
                 sshc->my->wait_on_socket();
                 rc = 0;
                 continue;
               }
             } else {
               char* msg;
               rc   = libssh2_session_last_error( sshc->my->m_session, &msg, 0, 0 );
               MACE_SSH_THROW( "write failed: %1% - %2%", %rc %msg  );
               return buf-data;
             }
           }
       } while( rc >= 0 && buflen);
       return buf-data;
    }
    int process_d::read_some( char* data, size_t len, int stream_id ) {
       int rc;
       char* buf = data;
       size_t buflen = len;
       do {
           rc = libssh2_channel_read_ex( chan, stream_id, buf, buflen );
           if( rc > 0 ) {
              buf += rc;
              buflen -= rc;
              return buf-data;
           } else if( rc == 0 ) {
              if( libssh2_channel_eof( chan ) )  {
                return -1; // eof
              }
           } else {
             if( rc == LIBSSH2_ERROR_EAGAIN ) {
               if( 0 < (buf-data) ) {
                 return buf-data;
               }
               else  {
                 sshc->my->wait_on_socket();
                 rc = 0;
                 continue;
               }
             } else {
               char* msg;
               rc   = libssh2_session_last_error( sshc->my->m_session, &msg, 0, 0 );
               MACE_SSH_THROW( "read failed: %1% - %2%", %rc %msg  ); return buf-data;
             }
           }
       } while( rc >= 0 && buflen);
       return buf-data;
    }
  
    
    /**
     *  Perform a blocking read.
     */
    std::streamsize process_source::read( char* s, std::streamsize n ) {
        char* buf = s;
        std::streamsize buflen = n;
        std::streamsize r = m_process.read_some( buf, buflen, m_chan );
        while( buflen ) {
          if( r == -1 ) {
            if( buf == s ) return -1;
            return (buf-s);
          }
          buf    += r;
          buflen -= r;
          r = m_process.read_some( buf, buflen, m_chan );
        }
        return n;
    }

    /**
     *  Perform a blocking write
     */
    std::streamsize process_sink::write( const char* s, std::streamsize n ) {
        const char* buf = s;
        std::streamsize buflen = n;
        std::streamsize r = m_process.write_some( buf, buflen, m_chan );
        while( buflen ) {
          if( r > 0 ) {
              buf += r;
              buflen -= r;
          }
        }
        return m_process.write_some( s, n, m_chan );
    }
    bool process_sink::flush( ) {
      return m_process.flush( m_chan );
    }

    void process_sink::close() {
      m_process.send_eof();
    }


  } // namespace detail


  process::process( client& c, const std::string& cmd )
  :my( new detail::process_d( c, cmd ) ){}

  process::~process() { 
    try {
      if( my->chan ) {
          int ec = libssh2_channel_free( my->chan );  
          while ( ec == LIBSSH2_ERROR_EAGAIN ) {
              my->sshc->my->wait_on_socket();
            ec = libssh2_channel_free( my->chan );  
          }
      }
    } catch ( ... ) { }
    delete my; 
  }

  /**
   *  This method will block until the remote channel is closed before
   *  it can return the exit code.
   *
   *  @pre client has not been freed.
   */
  int process::result() {
    char* msg = 0;
    int ec = libssh2_channel_wait_closed( my->chan );
    while( ec == LIBSSH2_ERROR_EAGAIN ) {
      my->sshc->my->wait_on_socket();
      ec = libssh2_channel_wait_closed( my->chan );
    }
    ec   = libssh2_session_last_error( my->sshc->my->m_session, &msg, 0, 0 );
    if( !ec ) {
      MACE_SSH_THROW( "Error waiting on socket to close: %1% - %2%", %ec %msg );
    }
    return libssh2_channel_get_exit_status( my->chan );
  }


  detail::io::stream<detail::process_sink>& process::in_stream()  { return my->std_in;  }
  std::istream& process::out_stream() { return my->std_out; }
  std::istream& process::err_stream() { return my->std_err; }

  



} } // namespace mace::ssh
