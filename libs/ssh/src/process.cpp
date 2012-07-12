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
      process_d( mace::ssh::client& c, const std::string& cmd, const std::string&  pty_type )
      :sshc(c.shared_from_this()),
       std_out(process_source(*this,0)),
       std_err(process_source(*this,SSH_EXTENDED_DATA_STDERR)),
       std_in(process_sink(*this,0) ) 
       {
        BOOST_ASSERT( c.my->m_session );

        chan = c.my->open_channel(pty_type); 

        /*
        unsigned int rw_size = 0;
        int ec = libssh2_channel_receive_window_adjust2(chan, 1024*64, 0, &rw_size );
        while( ec == LIBSSH2_ERROR_EAGAIN ) {
          sshc->my->wait_on_socket();
          ec = libssh2_channel_receive_window_adjust2(chan, 1024*64, 0, &rw_size );
        }
        elog( "rwindow size %1%", rw_size );
        */


        int ec = libssh2_channel_handle_extended_data2(chan, LIBSSH2_CHANNEL_EXTENDED_DATA_NORMAL );
        while( ec == LIBSSH2_ERROR_EAGAIN ) {
          sshc->my->wait_on_socket();
          ec = libssh2_channel_handle_extended_data2(chan, LIBSSH2_CHANNEL_EXTENDED_DATA_NORMAL );
        }

        if( cmd.size() == 0 ) {
            ec = libssh2_channel_shell(chan );
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              sshc->my->wait_on_socket();
              ec = libssh2_channel_shell(chan);
            }
        } else {
            ec = libssh2_channel_exec( chan, cmd.c_str() );
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              sshc->my->wait_on_socket();
              ec = libssh2_channel_exec( chan, cmd.c_str() );
            }
        }
        if( ec ) {
           char* msg = 0;
           ec   = libssh2_session_last_error( sshc->my->m_session, &msg, 0, 0 );
           MACE_SSH_THROW( "libssh2_channel_exec failed: %1% - %2%", %ec %msg  );
        }
      }


      void send_eof() {
        if( sshc->my->m_session ) {
          int ec = libssh2_channel_send_eof( chan );
          while( ec == LIBSSH2_ERROR_EAGAIN ) {
            sshc->my->wait_on_socket();
            ec = libssh2_channel_send_eof( chan );
          }
          if( ec ) {
            char* msg = 0;
            ec = libssh2_session_last_error( sshc->my->m_session, &msg, 0, 0 );
            MACE_SSH_THROW( "send eof failed: %1% - %2%", %ec %msg  );
          }
        }
      }
      bool flush() {
        int ec = libssh2_channel_flush_ex( chan, LIBSSH2_CHANNEL_FLUSH_EXTENDED_DATA);
        while( ec == LIBSSH2_ERROR_EAGAIN ) {
          sshc->my->wait_on_socket();
          ec = libssh2_channel_flush_ex( chan, LIBSSH2_CHANNEL_FLUSH_EXTENDED_DATA );
        }
        if( ec < 0 ) {
          char* msg = 0;
          libssh2_session_last_error( sshc->my->m_session, &msg, 0, 0 );
          elog( "flush failed: %1% - %2%", ec, msg  );
          MACE_SSH_THROW( "flush failed: %1% - %2%", %ec %msg  );
        }
        return true;
      }
    };

    int process_d::write_some( const char* data, size_t len, int stream_id ) {
        if( !sshc->my->m_session ) {
          elog( "SESSION CLOSED" );
          MACE_SSH_THROW( "Session closed\n" );
        }
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
                 elog( "return %1%", -1 );
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
        if( !sshc->my->m_session ) {
          MACE_SSH_THROW( "Session closed\n" );
        }
       
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
              sshc->my->wait_on_socket();
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
        return m_process.read_some( s, n, m_chan );
    }

    /**
     *  Perform a blocking write
     */
    std::streamsize process_sink::write( const char* s, std::streamsize n ) {
        try {
         return m_process.write_some( s, n, m_chan );
        } catch ( ... ) {
          elog( "%1%", boost::current_exception_diagnostic_information() );
          throw;
        }
    }
    bool process_sink::flush( ) {
      try {
        return m_process.flush();
      } catch ( ... ) {
        elog( "%1%", boost::current_exception_diagnostic_information() );
        throw;
      }
    }

    void process_sink::close() {
      m_process.send_eof();
    }


  } // namespace detail


  process::process( client& c, const std::string& cmd, const std::string& pty_type )
  :my( new detail::process_d( c, cmd, pty_type ) ){}

  process::~process() { 
    try {
      BOOST_ASSERT( !my->result );
      if( my->sshc->my->m_session ) {
        if( my->chan ) {
            int ec = libssh2_channel_free( my->chan );  
            while ( ec == LIBSSH2_ERROR_EAGAIN ) {
                my->sshc->my->wait_on_socket();
              ec = libssh2_channel_free( my->chan );  
            }
        }
        my->chan = 0;
      } 
    }catch ( ... ) {elog("error %1%", boost::current_exception_diagnostic_information() ); }
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

    int ec = libssh2_channel_wait_eof( my->chan );
    while( ec == LIBSSH2_ERROR_EAGAIN ) {
      my->sshc->my->wait_on_socket();
      ec = libssh2_channel_wait_eof( my->chan );
    }

    ec = libssh2_channel_wait_closed( my->chan );
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
